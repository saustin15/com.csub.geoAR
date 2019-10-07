//
//  MarkerTrackable.cpp
//  ArchitectCore
//
//  Created by Andreas Schacherbauer on 01.12.18.
//  Copyright © 2018 Wikitude. All rights reserved.
//

#include "MarkerTrackable.hpp"

#include "MarkerTrackerJavaScriptPluginModule.hpp"


MarkerTrackable::MarkerTrackable(long id_, long markerTrackerId_, int markerId_)
:
_id(id_),
_markerTrackerId(markerTrackerId_),
_markerId(markerId_),
_positionableId("com.wikitude.plugins.marker_tracker_demo.positionable_" + std::to_string(markerId_)),
_state(State::Lost)
{ /* Intentionally Left Blank */ }

void MarkerTrackable::processTrackedMarker(int markerId_, wikitude::sdk::Matrix4& matrix_, std::unordered_map<std::string, demo::PositionableData>& positionableData_) {
    
    if ( _markerId != markerId_ ) {
        return;
    }

    if ( _state == State::Lost ) {
        _state = State::Recognition;
        positionableData_.emplace(std::piecewise_construct, std::forward_as_tuple(_positionableId), std::forward_as_tuple(_id, demo::PositionableState::Recognized, markerId_, matrix_));
    } else {
        _state = State::Tracking;
        positionableData_.emplace(std::piecewise_construct, std::forward_as_tuple(_positionableId), std::forward_as_tuple(_id, demo::PositionableState::Tracking, markerId_, matrix_));
    }
}

void MarkerTrackable::cleanupRecentMarkers(int markerId_, std::unordered_map<std::string, demo::PositionableData>& positionableData_) {

    if ( _markerId != markerId_ ) {
        return;
    }

    _state = State::Lost;
    wikitude::sdk::Matrix4 identity;
    positionableData_.emplace(std::piecewise_construct, std::forward_as_tuple(_positionableId), std::forward_as_tuple(_id, demo::PositionableState::Lost, markerId_, identity));
}
