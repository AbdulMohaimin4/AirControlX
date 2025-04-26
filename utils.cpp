// utils.cpp

#include "utils.hpp"

std::string toString(FlightType type) {

    switch (type) {
        
        case FlightType::Commercial: return "Commercial";
        case FlightType::Cargo: return "Cargo";
        case FlightType::Military: return "Military";
        case FlightType::Medical: return "Medical";
    }
    return "Unknown";
}

std::string toString(AircraftPhase phase) {

    switch (phase) {

        case AircraftPhase::Holding: return "Holding";
        case AircraftPhase::Approach: return "Approach";
        case AircraftPhase::Landing: return "Landing";
        case AircraftPhase::Taxi: return "Taxi";
        case AircraftPhase::AtGate: return "At Gate";
        case AircraftPhase::Takeoff: return "Takeoff";
        case AircraftPhase::Climb: return "Climb";
        case AircraftPhase::Cruise: return "Cruise";
    }
    return "Unknown";
}

std::string toString(AirlineName airline) {

    switch (airline) {

        case AirlineName::AghaKhan_Air_Ambulance: return "AghaKhan Air Ambulance";
        case AirlineName::AirBlue: return "AirBlue";
        case AirlineName::Blue_Dart: return "Blue Dart";
        case AirlineName::FedEx: return "FedEx";
        case AirlineName::Pakistan_Airforce: return "Pakistan Airforce";
        case AirlineName::PIA: return "PIA";
    }
    return "Unknown";
}