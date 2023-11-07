/*
	THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
				http://dev-c.com			
			(C) Alexander Blade 2015
*/

#include <main.h>
#include "script.h"
#include "keyboard.h"
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

Ped playerPed;
BOOL playerExists;

bool notificationShown = false;
int stealAttempts = 0;
Vehicle lastStolenVehicle = NULL;

void ShowNotification(const char* message) {
    char msg[256];
    strncpy_s(msg, message, sizeof(msg));
    msg[sizeof(msg) - 1] = '\0';
    char* msgPtr = msg;
    UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
    UI::_ADD_TEXT_COMPONENT_STRING(msgPtr);
    UI::_DRAW_NOTIFICATION(FALSE, TRUE);
}

bool IsPlayerNearbyVehicleWithPedInside() {
    // Get the player's position and check if there's a vehicle with a ped inside nearby
    Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(PLAYER::PLAYER_PED_ID(), true);
    float radius = 5.0; // Adjust the radius as needed
    Vehicle nearbyVehicle = NULL;
    Ped pedInVehicle = NULL;

    // Find the nearest vehicle with a ped inside
    bool foundNearbyVehicle = false;
    for (int i = 0; i < 50; i++) {
        nearbyVehicle = PED::GET_VEHICLE_PED_IS_TRYING_TO_ENTER(PLAYER::PLAYER_PED_ID());
        if (ENTITY::DOES_ENTITY_EXIST(nearbyVehicle)) {
            pedInVehicle = VEHICLE::GET_PED_IN_VEHICLE_SEAT(nearbyVehicle, -1);
            if (ENTITY::DOES_ENTITY_EXIST(pedInVehicle)) {
                if (nearbyVehicle != lastStolenVehicle) { // Check if it's not the last stolen vehicle
                    foundNearbyVehicle = true;
                    break;
                }
            }
        }
        WAIT(100);
    }

    return foundNearbyVehicle;
}

bool IsRandomVehicleUnlocked() {
    int randomValue = rand() % 2; // 0 or 1
    return randomValue == 1; // 1 means unlocked, 0 means locked
}

bool IsVehicleMotorcycle(Vehicle vehicle) {
    // Identify motorcycles by their model hash
    int model = ENTITY::GET_ENTITY_MODEL(vehicle);
    return (model == GAMEPLAY::GET_HASH_KEY("bagger") || model == GAMEPLAY::GET_HASH_KEY("faggio"));
}

bool IsQuad(Vehicle vehicle) {
    // Identify quads by their model hash
    int model = ENTITY::GET_ENTITY_MODEL(vehicle);
    return (model == GAMEPLAY::GET_HASH_KEY("blazer") || model == GAMEPLAY::GET_HASH_KEY("blazer2") || model == GAMEPLAY::GET_HASH_KEY("blazer3"));
}

void LockVehicle(Vehicle vehicle) {
    if (ENTITY::DOES_ENTITY_EXIST(vehicle) && VEHICLE::IS_VEHICLE_DRIVEABLE(vehicle, false) && !IsVehicleMotorcycle(vehicle) && !IsQuad(vehicle)) {
        VEHICLE::SET_VEHICLE_DOORS_LOCKED(vehicle, 2); // Lock the vehicle
    }
}

void BreakIntoLockedVehicle(Vehicle vehicle) {
    if (ENTITY::DOES_ENTITY_EXIST(vehicle) && VEHICLE::IS_VEHICLE_DRIVEABLE(vehicle, false)) {
        VEHICLE::SET_VEHICLE_DOORS_LOCKED(vehicle, 0); // Break into the locked vehicle
    }
}

void ScriptMain() {
    ULONGLONG(GetTickCount64());

    // Seed the random number generator
    srand(static_cast<unsigned int>(time(0)));

    while (true) {
        if (IsKeyJustUp(0x46)) { // Use 0x46 for "F" key
            if (!notificationShown) {
                if (IsPlayerNearbyVehicleWithPedInside()) {
                    Vehicle nearbyVehicle = PED::GET_VEHICLE_PED_IS_TRYING_TO_ENTER(PLAYER::PLAYER_PED_ID());

                    // Check if the nearbyVehicle is not a motorcycle
                    if (!IsVehicleMotorcycle(nearbyVehicle)) {
                        if (IsRandomVehicleUnlocked()) {
                            // ShowNotification("You successfully stole an unlocked vehicle!");
                        }
                        else {
                            stealAttempts++;
                            if (stealAttempts == 2) {
                                // ShowNotification("You break into the locked vehicle!");
                                BreakIntoLockedVehicle(nearbyVehicle);
                                stealAttempts = 0; // Reset the attempts
                            }
                            else {
                               // ShowNotification("The vehicle is locked. You couldn't steal it.");
                                LockVehicle(nearbyVehicle); // Lock the vehicle
                            }
                        }
                        lastStolenVehicle = nearbyVehicle; // Set the last stolen vehicle
                        notificationShown = true;
                    }
                }
            }
        }
        else {
            notificationShown = false;
        }

        WAIT(0);
    }
}

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        scriptRegister(hInstance, ScriptMain);
        keyboardHandlerRegister(OnKeyboardMessage);
        break;
    case DLL_PROCESS_DETACH:
        scriptUnregister(hInstance);
        keyboardHandlerUnregister(OnKeyboardMessage);
        break;
    }
    return TRUE;
}