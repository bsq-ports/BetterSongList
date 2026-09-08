#include "beatsaber-hook/shared/safeptr.hpp"
#include "Patches/HookLevelCollectionTableSet.hpp"
#include "Patches/HookSelectedCollection.hpp"
#include "Patches/HookSelectedCategory.hpp"
#include "Patches/HookSelectedInTable.hpp"
#include "Utils/PlaylistUtils.hpp"
#include "config.hpp"
#include "logging.hpp"

#include "System/Threading/Tasks/Task.hpp"
#include "System/Threading/Tasks/Task_1.hpp"
#include "System/Threading/Tasks/TaskScheduler.hpp"
#include "System/Threading/CancellationToken.hpp"
#include "System/Threading/CancellationTokenSource.hpp"

#include "TMPro/TextMeshProUGUI.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Rect.hpp"
#include "UnityEngine/RectTransform.hpp"
#include "HMUI/AlphabetScrollbar.hpp"
#include "GlobalNamespace/AlphabetScrollInfo.hpp"

#include "bsml/shared/BSML/MainThreadScheduler.hpp"
#include "bsml/shared/BSML/SharedCoroutineStarter.hpp"

#include "UI/FilterUI.hpp"
#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cxxabi.h>
#include <optional>
#include <sstream>
#include <future>
#include "songcore/shared/SongCore.hpp"


namespace BetterSongList::Hooks {
    static ArrayW<GlobalNamespace::BeatmapLevel*> CloneLevels(ArrayW<GlobalNamespace::BeatmapLevel*> levels) {
        if (!levels) return nullptr;
        ArrayW<GlobalNamespace::BeatmapLevel*> copy(levels.size());
        std::copy(levels.begin(), levels.end(), copy.begin());
        return copy;
    }

    static safe_ptr<GlobalNamespace::LevelCollectionTableView*> lastTableView;

    ISorter* HookLevelCollectionTableSet::sorter;
    IFilter* HookLevelCollectionTableSet::filter;
    std::function<void(ArrayW<GlobalNamespace::BeatmapLevel*>)> HookLevelCollectionTableSet::recallLast;
    safe_ptr<ArrayW<GlobalNamespace::BeatmapLevel*>> HookLevelCollectionTableSet::lastInMapList;
    safe_ptr<ArrayW<GlobalNamespace::BeatmapLevel*>> HookLevelCollectionTableSet::lastOutMapList;
    safe_ptr<ArrayW<GlobalNamespace::BeatmapLevel*>> HookLevelCollectionTableSet::asyncPreProcessed;
    ISorterWithLegend::Legend HookLevelCollectionTableSet::customLegend;
    bool HookLevelCollectionTableSet::prepareThreadCurrentlyRunning = false;
    bool HookLevelCollectionTableSet::tryReselectLastSelectedLevel = false;

    ArrayW<GlobalNamespace::BeatmapLevel*> HookLevelCollectionTableSet::get_lastInMapList() {
        if (!lastInMapList) {
            return nullptr;
        }
        return lastInMapList.ptr();
    }

    ArrayW<GlobalNamespace::BeatmapLevel*> HookLevelCollectionTableSet::get_lastOutMapList() {
        if (!lastOutMapList) {
            return nullptr;
        }
        return lastOutMapList.ptr();
    }

    /**
     * @brief Refresh the SongList with the last used BeatMaps array
     * 
     * @param processAsync 
     * @param clearAsyncResult 
     */
    void HookLevelCollectionTableSet::Refresh(bool processAsync, bool clearAsyncResult) {
        if (!lastTableView || !lastTableView->get_isActiveAndEnabled() || !get_lastInMapList()) {
            return;
        }

        DEBUG("Refresh({}, {})", processAsync, clearAsyncResult);

        /*
        * This probably has problems in regards to race conditions / thread safety... We will see...
        * Pre-processes the desired songlist state in a second thread - This will then get stored in
        * a vaiable and used as the result on the next SetData() in the Prefix hook below
        */
        if (clearAsyncResult) {
            asyncPreProcessed.emplace(nullptr);
        }

        if (processAsync) {
            PrepareStuffIfNecessary([](){
                // Sorting mutates the array, so preserve the cached input order.
                auto inList = CloneLevels(get_lastInMapList());

                FilterWrapper(inList);
                asyncPreProcessed.emplace(inList);
                Refresh(false, false);
            }, true);
            return;
        }

        /*
        * Forcing a refresh of the table by skipping the optimization check in the SetData():Prefix
        * because Refresh() is only called in situations where the result will probably change
        */
        auto ml = lastInMapList;
        lastInMapList.emplace(nullptr);
        // SetData replaces recallLast; keep this invocation and its captures alive.
        auto recall = recallLast;
        if (recall) recall(ml.ptr());
    }

    void HookLevelCollectionTableSet::FilterWrapper(ArrayW<GlobalNamespace::BeatmapLevel*>& previewBeatmapLevels) {
        if (!previewBeatmapLevels) {
            return;
        }

        // If SongCore didn't load, don't do anything yet
        // TODO: Make it thread safe instead and discard the result or implement loading state
        if (!SongCore::API::Loading::AreSongsLoaded()) {
            return;
        }

        if ((filter && !filter->get_isReady()) && (sorter && !sorter->get_isReady())) return;

        DEBUG("FilterWrapper({})", previewBeatmapLevels.size());

        if (filter && filter->get_isReady()) {
            INFO("Filtering levels");
            auto* const activeFilter = filter;
            std::vector<GlobalNamespace::BeatmapLevel*> filtered;
            filtered.reserve(previewBeatmapLevels.size());
            for (auto level : previewBeatmapLevels) {
                if (activeFilter->GetValueFor(level)) filtered.push_back(level);
            }
            previewBeatmapLevels = ArrayW<GlobalNamespace::BeatmapLevel*>(filtered);
        }

        INFO("We're down to {}", previewBeatmapLevels.size());
        if (sorter && sorter->get_isReady()) {
            INFO("Sorting levels!");
            if (auto* customSorter = sorter->as<ISorterCustom*>()) {
                customSorter->DoSort(previewBeatmapLevels, config.get_sortAsc());
            }
            else if (auto* primitiveSorter = sorter->as<ISorterPrimitive*>()) {
                // sorting is the same regardless of ascending or descending, since the way we differentiate between ascending and descending is to just use the reverse iterators if ascending
                auto sort = [primitiveSorter]
                (GlobalNamespace::BeatmapLevel* lhs, GlobalNamespace::BeatmapLevel* rhs) -> bool {
                    return primitiveSorter->GetValueFor(lhs) < primitiveSorter->GetValueFor(rhs);
                };

                if (config.get_sortAsc()) std::sort(previewBeatmapLevels.rbegin(), previewBeatmapLevels.rend(), sort);
                else std::sort(previewBeatmapLevels.begin(), previewBeatmapLevels.end(), sort);
            }
            // if it was neither, print the type name
            else {
                ISorter& s = *sorter;
                auto& ti = typeid(s);
                int status;
                auto realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
                ERROR("Sorter was of type {} which is not a valid sorter type!", realname);
                free(realname);
            }
        }

        auto withLegend = sorter ? sorter->as<ISorterWithLegend*>() : nullptr;
        if (withLegend && sorter->get_isReady()) {
            INFO("Sorter had legend!");
            customLegend = withLegend->BuildLegend(previewBeatmapLevels);
        } else {
            customLegend.clear();
        }

        INFO("Ended with {} levels in array", previewBeatmapLevels.size());
    }


    static std::shared_ptr<std::atomic_bool> doCancelSort;

    bool HookLevelCollectionTableSet::PrepareStuffIfNecessary(std::function<void()> cb, bool cbOnAlreadyPrepared) {
        INFO("PrepareStuffIfNecessary({}, {})", cb != nullptr, cbOnAlreadyPrepared);

        // A ready request also supersedes any preparation that was queued earlier.
        if (doCancelSort) {
            doCancelSort->store(true, std::memory_order_release);
            doCancelSort.reset();
        }

        if ((filter && !filter->get_isReady()) || (sorter && !sorter->get_isReady())) {
            auto instance = FilterUI::get_instance();
            auto indicator = instance->filterLoadingIndicator;
            if (indicator && indicator->___m_CachedPtr.m_value) {
                indicator->get_gameObject()->SetActive(true);
            }

            doCancelSort = std::make_shared<std::atomic_bool>(false);
            DEBUG("PrepareStuffIfNecessary()");
            std::thread([cb, view = lastTableView, activeSorter = sorter, activeFilter = filter](std::weak_ptr<std::atomic_bool> thisDoCancelSort){
                if (activeSorter && !activeSorter->get_isReady()) {
                    activeSorter->Prepare().wait();
                }

                if (activeFilter && !activeFilter->get_isReady()) {
                    activeFilter->Prepare().wait();
                }

                DEBUG("ContinueWith");
                auto cancelSort = thisDoCancelSort.lock();
                if (!cancelSort) {
                    INFO("sort was cancelled by invalidated weak_ptr");
                    return;
                }

                if (!cancelSort->load(std::memory_order_acquire) && cb) {
                    BSML::MainThreadScheduler::Schedule([cb, thisDoCancelSort, view] {
                        auto cancelSort = thisDoCancelSort.lock();
                        if (!cancelSort || cancelSort->load(std::memory_order_acquire)) return;
                        if (!view || !lastTableView || view.ptr() != lastTableView.ptr() || !view->get_isActiveAndEnabled()) return;
                        cb();
                    });
                }
            }, doCancelSort).detach();

            return true;
        } else {
            INFO("Sorter & Filter were prepared!");
        }

        if (cbOnAlreadyPrepared && cb) cb();
        return false;
    }

    void HookLevelCollectionTableSet::LevelCollectionTableView_SetData_Prefix(GlobalNamespace::LevelCollectionTableView* self, ArrayW<GlobalNamespace::BeatmapLevel*>& previewBeatmapLevels, HashSet<StringW>* favoriteLevelIds, bool& beatmapLevelsAreSorted) {
        DEBUG("LevelCollectionTableView.SetData() : Prefix");

        // If SetData is called with the literal same maplist as before we might as well ignore it
        if (lastTableView && lastTableView.ptr() == self && get_lastInMapList() && previewBeatmapLevels.convert() == get_lastInMapList().convert() && get_lastOutMapList()) {
            DEBUG("LevelCollectionTableView.SetData() : Prefix -> levels = lastout because {} == {}", previewBeatmapLevels.convert(), get_lastInMapList().convert());
            previewBeatmapLevels = get_lastOutMapList();
            return;
        }

        lastTableView.emplace(self);

        // Playlistlib has its own custom wrapping class for Playlists so it can properly track duplicates, so we need to use its collection
        if (HookSelectedCollection::get_lastSelectedCollection() && PlaylistUtils::get_hasPlaylistLib()) {
            auto playlistArr = PlaylistUtils::GetLevelsForLevelCollection(HookSelectedCollection::get_lastSelectedCollection());
            if (playlistArr) {
                previewBeatmapLevels = playlistArr;
            }
        }

        // Keep a separate input array because filtering and sorting replace or reorder the output.
        lastInMapList.emplace(CloneLevels(previewBeatmapLevels));

        // This is a callback to call the sort again with the same parameters
        auto isSorted = beatmapLevelsAreSorted;
        recallLast = [view = safe_ptr<GlobalNamespace::LevelCollectionTableView*>(self), favorites = safe_ptr<HashSet<StringW>*>(favoriteLevelIds), isSorted](ArrayW<GlobalNamespace::BeatmapLevel *> overrideData){
            if (!view || !view->get_isActiveAndEnabled()) return;
            tryReselectLastSelectedLevel = true;
            auto data = overrideData ? overrideData : get_lastInMapList();
            INFO("recallLast, Data: {}", data.convert());
            if (data) {
                INFO("Setting data with {} levels", data.size());
            }
            view->SetData((System::Collections::Generic::IReadOnlyList_1<GlobalNamespace::BeatmapLevel*>*)data.convert(), favorites.ptr(), isSorted, !isSorted);
        };

        // If this is true the default Alphabet scrollbar is processed / shown - We dont want that when we use a custom filter
        if (!sorter || sorter->get_isReady()) {
            beatmapLevelsAreSorted = false;
            DEBUG("We have to sort!");
        }

        if (PrepareStuffIfNecessary([](){Refresh(true);})) {
            DEBUG("Stuff isnt ready yet... Preparing it and then reloading list: Sorter {0}, Filter {1}", !sorter || sorter->get_isReady(), !filter || filter->get_isReady());
        }

        auto instance = FilterUI::get_instance();
        auto loadingIndicator = instance->filterLoadingIndicator;
        if (loadingIndicator && loadingIndicator->___m_CachedPtr.m_value) {
            loadingIndicator->get_gameObject()->SetActive(false);
        }

        if (asyncPreProcessed && asyncPreProcessed.ptr()) {
            previewBeatmapLevels = asyncPreProcessed.ptr();
			DEBUG("Used Async-Prefiltered");
            asyncPreProcessed.emplace(nullptr);
            return;
        }

        // Keep sorting from changing the array owned by the selected pack or caller.
        previewBeatmapLevels = CloneLevels(previewBeatmapLevels);
        FilterWrapper(previewBeatmapLevels);
    }

    static custom_types::Helpers::Coroutine TryReselectLastSelectedSong(safe_ptr<GlobalNamespace::LevelCollectionTableView*> view) {
        // Skip a frame
        co_yield nullptr;

        if (!view || !lastTableView || view.ptr() != lastTableView.ptr() || !view->get_isActiveAndEnabled()) co_return;
        auto* __instance = view.ptr();
        auto lastOutMapList = HookLevelCollectionTableSet::get_lastOutMapList();
        int lastOutSize = lastOutMapList ? lastOutMapList.size() : 0;
        if(
            __instance == nullptr || 
            __instance->m_CachedPtr.m_value == nullptr || 
            lastOutSize == 0
        ) {
            co_return;
        }
        auto level = lastOutMapList.front_or_default([](GlobalNamespace::BeatmapLevel* level) {
            return level && level->___levelID == config.get_lastSong();
        });
        if (!level) {
            WARNING("LevelCollectionTableView.SetData():Postfix => TryReselectLastSelectedSong: No last selected song found, skipping reselect");
            co_return;
        }
        int levelIndex = -1;
        if (level) {
            std::optional<int> resultindex = lastOutMapList.index_of(level);
            if (resultindex.has_value()) {
                levelIndex = resultindex.value();
            }
        }
        if (levelIndex == -1) {
            WARNING("LevelCollectionTableView.SetData():Postfix => TryReselectLastSelectedSong: No last selected song found, skipping reselect");
            co_return;
        }

        int idx = std::max(0, levelIndex + (__instance->_showLevelPackHeader ? 1 : 0));
        DEBUG("LevelCollectionTableView.SetData():Postfix => TryReselectLastSelectedSong: Scrolling to song with idx {}",  idx);
        __instance->_selectedRow = idx;
        __instance->_tableView->SelectCellWithIdx(idx, false);
        __instance->_tableView->ScrollToCellWithIdx(idx, HMUI::TableView::ScrollPositionType::Center, false);
    }

    void HookLevelCollectionTableSet::LevelCollectionTableView_SetData_PostFix(GlobalNamespace::LevelCollectionTableView* self, ArrayW<GlobalNamespace::BeatmapLevel*> previewBeatmapLevels) {
        DEBUG("HookLevelCollectionTableSet::PostFix({}, {})", fmt::ptr(self), previewBeatmapLevels ? previewBeatmapLevels.size() : 0);
        lastOutMapList.emplace(static_cast<Array<GlobalNamespace::BeatmapLevel*>*>(previewBeatmapLevels));

        if(tryReselectLastSelectedLevel) {
            BSML::SharedCoroutineStarter::get_instance()->StartCoroutine(custom_types::Helpers::CoroutineHelper::New(TryReselectLastSelectedSong(self)));
            tryReselectLastSelectedLevel = false;
        }

        // Basegame already handles cleaning up the legend etc
        if (customLegend.empty()) {
            // TODO: Base game issue. Remove when fixed. (Idk, ported cause it was in the original code)
            if (previewBeatmapLevels.size() == 0) {
                self->____alphabetScrollbar->get_gameObject()->SetActive(false);
            }
            return;
        };

        /*
        * We essentially gotta double-init the alphabet scrollbar because basegame
        * made the great decision to unnecessarily lock down the scrollbar to only
        * use characters, not strings
        */
        auto alphabetScrollBar = self->____alphabetScrollbar;
        auto data = ArrayW<GlobalNamespace::AlphabetScrollInfo::Data*>(il2cpp_array_size_t(customLegend.size()));
        DEBUG("Legend size: {}, {}", data.size(), customLegend.size());
        for (int i = 0; const auto& [key, value] : customLegend)
            data[i++] = GlobalNamespace::AlphabetScrollInfo::Data::New_ctor(u'?', value);
        DEBUG("Setting data");
        alphabetScrollBar->SetData(reinterpret_cast<System::Collections::Generic::IReadOnlyList_1<GlobalNamespace::AlphabetScrollInfo::Data*>*>(data.convert()));

        // Now that all labels are there we can insert the text we want there...
        ListW<TMPro::TextMeshProUGUI*> texts{alphabetScrollBar->____texts};
        for (int i = 0; const auto& [key, value] : customLegend) {
            texts[i++]->set_text(key);
        }

        customLegend.clear();

        // Move the table a bit to the right to accomodate for alphabet scollbar (Basegame behaviour)
        auto tableViewT = self->____tableView->get_transform().try_cast<UnityEngine::RectTransform>();
        auto scrollBarT = alphabetScrollBar->get_transform().try_cast<UnityEngine::RectTransform>();
        tableViewT->set_offsetMin({scrollBarT->get_rect().get_size().x + 1.0f, 0.0f});
        alphabetScrollBar->get_gameObject()->SetActive(true);
        DEBUG("scroll bar active!");
    }
}
