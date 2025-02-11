#pragma once

#include "UnityEngine/Component.hpp"
#include "UnityEngine/Transform.hpp"
#include "logging.hpp"
#include <string>

/**
 * @brief Constructs the full hierarchical name of a Unity component.
 *
 * This function retrieves the full path name of a Unity `Component` in the hierarchy,
 * starting from the root `Transform` down to the given component's `Transform`.
 * The path segments are separated by forward slashes (`/`).
 *
 * @param component Pointer to the Unity `Component` whose full name is to be retrieved.
 *                  The component must have a valid `Transform`.
 *
 * @return A string containing the full hierarchical name of the component, with each
 *         level separated by a forward slash (`/`). Returns an empty string if the
 *         component or its `Transform` is `nullptr`.
 *
 * @note If the component or its `Transform` is `nullptr`, the function logs a debug
 *       message using `BSLLogger::Logger` and returns an empty string.
 */
inline std::string GetFullName(UnityEngine::Component* component) {
    // Check if the component or its Transform is null.
    if (component == nullptr || component->transform == nullptr) {
        BSLLogger::Logger.debug("Component or its Transform is null."); // Log a debug message.
        return ""; // Return an empty string if invalid.
    }

    // Start with the current Transform and initialize the full name.
    UnityEngine::Transform* current = component->transform;
    std::string fullName = current->name;

    // Traverse up the hierarchy to construct the full name.
    while (current->parent != nullptr) {
        current = current->parent; // Move to the parent Transform.
        fullName = std::string(current->name) + "/" + fullName; // Prepend the parent's name.
    }

    return fullName; // Return the constructed full hierarchical name.
}
