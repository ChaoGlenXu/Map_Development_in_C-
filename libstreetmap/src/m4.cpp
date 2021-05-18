#include "m4.h"
#include "m3.h"
#include "m2.h"
#include "m1.h"

//#include "m3.cpp"

#pragma once
#include <vector>
#include <algorithm>

double m4_find_path_between_intersections(const IntersectionIndex 
intersect_id_start, const IntersectionIndex intersect_id_end, const double turn_penalty);

// This routine takes in a vector of N deliveries (pickUp, dropOff
// intersection pairs), another vector of M intersections that
// are legal start and end points for the path (depots), a turn 
// penalty in seconds (see m3.h for details on turn penalties), 
// and the truck_capacity in pounds.
//
// The first vector 'deliveries' gives the delivery information.  Each delivery
// in this vector has pickUp and dropOff intersection ids and the weight (also
// in pounds) of the delivery item. A delivery can only be dropped-off after
// the associated item has been picked-up. 
// 
// The second vector 'depots' gives the intersection ids of courier company
// depots containing trucks; you start at any one of these depots and end at
// any one of the depots.
//
// This routine returns a vector of CourierSubpath objects that form a delivery route.
// The CourierSubpath is as defined above. The first street segment id in the
// first subpath is connected to a depot intersection, and the last street
// segment id of the last subpath also connects to a depot intersection.  The
// route must traverse all the delivery intersections in an order that allows
// all deliveries to be made with the given truck capacity. Addionally, a package
// should not be dropped off if you haven't picked it up yet.
//
// The start_intersection of each subpath in the returned vector should be 
// at least one of the following (a pick-up and/or drop-off can only happen at 
// the start_intersection of a CourierSubpath object):
//      1- A start depot.
//      2- A pick-up location (and you must specify the indices of the picked 
//                              up orders in pickup_indices)
//      3- A drop-off location. 
//
// You can assume that N is always at least one, M is always at least one
// (i.e. both input vectors are non-empty), and truck_capacity is always greater
// or equal to zero.
//
// It is legal for the same intersection to appear multiple times in the pickUp
// or dropOff list (e.g. you might have two deliveries with a pickUp
// intersection id of #50). The same intersection can also appear as both a
// pickUp location and a dropOff location.
//        
// If you have two pickUps to make at an intersection, traversing the
// intersection once is sufficient to pick up both packages, as long as the
// truck_capcity fits both of them and you properly set your pickup_indices in
// your courierSubpath.  One traversal of an intersection is sufficient to
// drop off all the (already picked up) packages that need to be dropped off at
// that intersection.
//
// Depots will never appear as pickUp or dropOff locations for deliveries.
//  
// If no valid route to make *all* the deliveries exists, this routine must
// return an empty (size == 0) vector.
std::vector<CourierSubpath> traveling_courier(
		            const std::vector<DeliveryInfo>& deliveries,
                            const std::vector<int>& depots, 
		            const float turn_penalty, 
		            const float truck_capacity) {
    
    std::vector<CourierSubpath> finalPath;
    std::vector<StreetSegmentIndex> currentPathSegments;
    int numDeliveries = deliveries.size();
    std::vector<int> packagesPossessed;
    std::vector<int> pickedUpPackages;
    CourierSubpath currentSubPath;
    IntersectionIndex shortestPickUp;
    IntersectionIndex shortestDepot;
    int bestInitialIndex;
    double shortestPickUpTime = 999999;
    double currentPickUpTime;
    double currentTruckWeight = 0 ;
    int currentIntersectionDeliveryIndex;
    
    
  
 
    //Heuristic 1: Greedy                    
    //Find geometrically shortest distance from any depot to any pickup
    for(int i = 0; i < depots.size(); i++) {
        for(int j = 0; j < numDeliveries; j++) {
            LatLon depotCoords = getIntersectionPosition(depots[i]);
            LatLon pickUpCoords = getIntersectionPosition(deliveries[j].pickUp);
            currentPickUpTime = find_distance_between_two_points(std::make_pair(depotCoords, pickUpCoords));
            //If a new best time is found, update corresponding pickup and depot
            if (currentPickUpTime < shortestPickUpTime) {
                shortestPickUpTime = currentPickUpTime;
                shortestPickUp = deliveries[j].pickUp;
                shortestDepot = depots[i];
                bestInitialIndex = j;
            }
        }
        
    }
     
    
    //////////////////////////glen's attemp for improving////////////////////////
  /*
    //Heuristic : find the best path by  //find_path_between_intersections                 
    // from any depot to any pickup
    //vector<StreetSegmentIndex> current_path;
    double current_time;
    for(int i = 0; i < depots.size(); i++) {
        for(int j = 0; j < numDeliveries; j++) {
            current_time= m4_find_path_between_intersections(depots[i], deliveries[j].pickUp, turn_penalty );
            //If a new best time is found, update corresponding pickup and depot
            if (current_time < shortestPickUpTime) {
                shortestPickUpTime = current_time;
                shortestPickUp = deliveries[j].pickUp;
                shortestDepot = depots[i];
                bestInitialIndex = j;
            }
        }
        
    }
    
*/
  /////////////glen's attemp for improving////////////////////////////////////////  
    
    
    
    
   // std::cout << "Initial pickup: " << bestInitialIndex << std::endl;
    currentSubPath.start_intersection = shortestDepot;
    currentSubPath.end_intersection = shortestPickUp;
    packagesPossessed.push_back(bestInitialIndex);
    currentIntersectionDeliveryIndex = bestInitialIndex;
    currentTruckWeight  = currentTruckWeight + deliveries[bestInitialIndex].itemWeight;
    currentSubPath.subpath = find_path_between_intersections(shortestDepot, shortestPickUp, turn_penalty);
    
    pickedUpPackages.push_back(bestInitialIndex);
    finalPath.push_back(currentSubPath);
    
    
    //Variables for loop initialization
    IntersectionIndex currentIntersection = shortestPickUp;
    IntersectionIndex currentDropOff;
    IntersectionIndex shortestNextPickUp;
    IntersectionIndex shortestDropOff;
    int bestIndex;
    int bestDropOffIndex;
    int possessedPackageIndex;
    double shortestNextPickUpDistance = 9999999;
    double shortestDropOffDistance = 999999;
    bool pickUpAtStart = true;
    
    
    //While not all packages have been picked up yet or a package is still in possession, keep obtaining paths
    while(   (pickedUpPackages.size() < deliveries.size() )    || (  packagesPossessed.size() != 0)    ) { 
         shortestNextPickUpDistance = 9999999;
         shortestDropOffDistance = 999999;
        //Checking for nearest legal pickup
        for (int j = 0; j < numDeliveries; j++) {
            auto it = std::find(pickedUpPackages.begin(), pickedUpPackages.end(), j); 
                                //Check if the pickup has already been reached before
            if(it == pickedUpPackages.end()) { //If it hasn't been reached, consider it in the pickup checks
                LatLon pickUpCoords = getIntersectionPosition(currentIntersection);
                LatLon nextPickUpCoords = getIntersectionPosition(deliveries[j].pickUp);
                double distance = find_distance_between_two_points(
                std::make_pair(pickUpCoords, nextPickUpCoords));
            
            
                //If new shortest distance is found that does not equal the current PickUp, update nearest legal pickup
                if(deliveries[j].pickUp != currentIntersection && distance < shortestNextPickUpDistance && 
                        (currentTruckWeight + deliveries[j].itemWeight <= truck_capacity))  {
                    shortestNextPickUpDistance = distance;
                    shortestNextPickUp = deliveries[j].pickUp;
                    bestIndex = j;       
                }
            }
        }
         
       //Checking for nearest legal dropoff (out of currently possessed packages)
        for(int j = 0; j < packagesPossessed.size(); j++) {
            LatLon pickUpCoords = getIntersectionPosition(currentIntersection);
            LatLon dropOffCoords = getIntersectionPosition(deliveries[packagesPossessed[j]].dropOff);
           double distance = find_distance_between_two_points(std::make_pair(pickUpCoords, dropOffCoords));
           if(distance < shortestDropOffDistance) {
               shortestDropOffDistance = distance;
               shortestDropOff = deliveries[packagesPossessed[j]].dropOff;
               bestDropOffIndex = packagesPossessed[j];
               possessedPackageIndex = j;
           }
            
        }
        //Compare nearest legal pickup and dropoff, see which one is faster
           
           //If extra pickup is faster, set next subpath to pick up the extra pickup
           if(shortestNextPickUpDistance < shortestDropOffDistance) {
            //   std::cout << "Next pickup: " << bestIndex << std::endl;
               currentSubPath.start_intersection = currentIntersection; 
               currentSubPath.end_intersection = deliveries[bestIndex].pickUp;
               currentTruckWeight = currentTruckWeight + deliveries[bestIndex].itemWeight;
               currentSubPath.subpath = find_path_between_intersections(currentIntersection, 
                       deliveries[bestIndex].pickUp, turn_penalty);
               if(pickUpAtStart) { //If a package has been picked up at the start 
                                   //of this path, add it to the subpath's pickup indices
                   currentSubPath.pickUp_indices.push_back(currentIntersectionDeliveryIndex);
          //         std::cout << "Pickup detected for delivery " <<  currentIntersectionDeliveryIndex  << std::endl;
               }  
               pickedUpPackages.push_back(bestIndex);
               packagesPossessed.push_back(bestIndex);
               pickUpAtStart = true;
               currentIntersection = currentSubPath.end_intersection; // Set current intersection 
                                        //to end of this subpath for next iteration
               currentIntersectionDeliveryIndex = bestIndex;
           }
           
           //Otherwise, drop off fastest package
           else{
                             // std::cout << "Next dropoff: " << bestDropOffIndex << std::endl;
               currentSubPath.start_intersection = currentIntersection;
               currentSubPath.end_intersection = deliveries[bestDropOffIndex].dropOff;
               currentTruckWeight = currentTruckWeight - deliveries[bestDropOffIndex].itemWeight;
               currentSubPath.subpath = find_path_between_intersections(currentIntersection,
                       deliveries[bestDropOffIndex].dropOff, turn_penalty);
               packagesPossessed.erase(packagesPossessed.begin()+ possessedPackageIndex);
               if(pickUpAtStart) { //If a package has been picked up at the 
                           //start of this path, add it to the subpath's pickup indices
                   currentSubPath.pickUp_indices.push_back(currentIntersectionDeliveryIndex);
               //    std::cout << "Pickup detected for delivery " <<  currentIntersectionDeliveryIndex  << std::endl;
               }
               pickUpAtStart = false;
               currentIntersection = currentSubPath.end_intersection;
               currentIntersectionDeliveryIndex = bestDropOffIndex;
           }
           
           finalPath.push_back(currentSubPath);
           currentSubPath.pickUp_indices.clear();
        }
         
    
    //When all deliveries are made, go to nearest depot
    double shortestEndingDistance = 99999;
    int bestDepot;
    for(int i = 0; i < depots.size(); i++) {
        LatLon currentCoords = getIntersectionPosition(currentIntersection);
        LatLon endingCoords = getIntersectionPosition(depots[i]);
        double distance = find_distance_between_two_points(std::make_pair(currentCoords, endingCoords));
        if (distance < shortestEndingDistance) {
            shortestEndingDistance = distance;
            bestDepot = i;
        }
    }
    
    currentSubPath.start_intersection = currentIntersection;
    currentSubPath.end_intersection = depots[bestDepot];
    currentSubPath.subpath = find_path_between_intersections(currentIntersection, 
                                                depots[bestDepot], turn_penalty);
    finalPath.push_back(currentSubPath);
    
    
    
    /*  Rubbish Implementation 
        //Start from depot 0, go to first pickup location
    currentSubPath.start_intersection = depots[0];
    currentSubPath.end_intersection = deliveries[0].pickUp;
    currentSubPath.subpath = find_path_between_intersections(depots[0], deliveries[0].pickUp, turn_penalty);
   
    finalPath.push_back(currentSubPath);
    
    //Pick up and drop off packages in order
    for (int i = 0; i < numDeliveries - 1; i++) {
        currentSubPath.start_intersection = deliveries[i].pickUp;
        currentSubPath.end_intersection = deliveries[i].dropOff;
        currentSubPath.subpath = find_path_between_intersections(deliveries[i].pickUp, deliveries[i].dropOff, turn_penalty);
        currentSubPath.pickUp_indices.push_back(i);
        finalPath.push_back(currentSubPath);
        
        
        
        currentSubPath.pickUp_indices.clear();
        currentSubPath.start_intersection = deliveries[i].dropOff;
        currentSubPath.end_intersection = deliveries[i+1].pickUp;
        currentSubPath.subpath = find_path_between_intersections(deliveries[i].dropOff, deliveries[i+1].pickUp, turn_penalty);
        finalPath.push_back(currentSubPath);
    }
    
    currentSubPath.start_intersection = deliveries[numDeliveries - 1].pickUp;
    currentSubPath.end_intersection = deliveries[numDeliveries - 1].dropOff;
    currentSubPath.subpath = find_path_between_intersections(deliveries[numDeliveries - 1].pickUp, deliveries[numDeliveries - 1].dropOff, turn_penalty);
    currentSubPath.pickUp_indices.push_back(numDeliveries -1);
    finalPath.push_back(currentSubPath);
    
    currentSubPath.pickUp_indices.clear();
    currentSubPath.start_intersection = deliveries[numDeliveries - 1].dropOff;
    currentSubPath.end_intersection = depots[0];
    currentSubPath.subpath = find_path_between_intersections(deliveries[numDeliveries - 1].dropOff, depots[0],turn_penalty);
    finalPath.push_back(currentSubPath);
    
     * */
    return finalPath;
}





/*
//Glen's attempt to improve the  implementation for m4's new dijkstra's algorithm 
double m4_find_path_between_intersections(const IntersectionIndex 
intersect_id_start, const IntersectionIndex intersect_id_end, const double turn_penalty){
    vector<StreetSegmentIndex> finalPath;
    vector<waveElement> waveFront;
    LatLon start;
    LatLon end = getIntersectionPosition(intersect_id_end);    
    double totalTime;
    //Initial vector setup for pathfinding intersection data
    vector<intersectionData> intersections; 
    int intersectionCount = getNumIntersections();
    intersections.resize(intersectionCount);
    double averageSpeedLimit = 0;
    double totalSpeedLimit = 0;
    
    for(int i = 0; i < getNumStreetSegments(); i++) {
        totalSpeedLimit = totalSpeedLimit + getInfoStreetSegment(i).speedLimit;
    }
    averageSpeedLimit = totalSpeedLimit / getNumStreetSegments();
    
    for(int i = 0; i < intersectionCount; i++) { //loop through all intersections
        intersections[i].reachingEdge = NO_EDGE;//-1 initialize all intersection to be the starting intersection
        intersections[i].travelTime = NO_TIME; //9999 initialize all travelTime to be very big
        start = getIntersectionPosition(i);
        double heuristic = find_distance_between_two_points(make_pair(start, end));
        intersections[i].travelTimeToDestination = heuristic * 1000 / (averageSpeedLimit*3600);
        intersections[i].visited = false; //initialize all intersection to be un-visited
    }
    
    //Initial setup: Get adjacent intersections to the starting intersection 
    //(excluding one-way the wrong way), calculate the travel time for each and store them in the wavefront
    intersections[intersect_id_start].travelTime = 0; //the time that needs to get to the first node is zero

    
    waveFront.push_back(waveElement(intersect_id_start, NO_EDGE, 0, 
    intersections[intersect_id_start].travelTimeToDestination));
    
    //Turn the current wavefront into a min heap
    make_heap(waveFront.begin(), waveFront.end(), compareTravelTime()); 
    
    while (!waveFront.empty()) { //if the wavefront is not empty, meaning there are nodes for us to check
        waveElement currentElement = waveFront.front(); //take the first element of the wavefront
        
        //Take the element with smallest travel time and remove it from the heap
        pop_heap(waveFront.begin(), waveFront.end(), compareTravelTime()); 
        waveFront.pop_back(); 
        
         
        if (intersections[currentElement.intersectionID].visited == false) {
            //Find outward edges of current intersection
            vector<int> currentElementSegments = find_street_segments_of_intersection(currentElement.intersectionID); 
            
            //Analyze outward edges one at a time, finding travel times to new nodes
            int currentSegmentNum = currentElementSegments.size();
            for (int i = 0; i < currentSegmentNum; i++) {
                InfoStreetSegment currentSegment = getInfoStreetSegment(currentElementSegments[i]);
                double finalTravelTime;
                
                double newTravelTime = find_street_segment_travel_time(currentElementSegments[i]);
                
                //check for turn penalty 
                //If a turn is detected, add the turn penalty
                if (turnPenaltyCheck(currentElement.edgeID, currentElementSegments[i])) { 
                    finalTravelTime = currentElement.travelTime + newTravelTime + turn_penalty; 
                } else {
                    //Total travel time is travel time of current intersection plus travel time of new segment
                    finalTravelTime = currentElement.travelTime + newTravelTime; 
                }
                
                
                if (currentElement.intersectionID == currentSegment.to)
                { 
                    totalTime = finalTravelTime + intersections[currentSegment.from].travelTimeToDestination;
                   //Only add to wavefront if newly founded travel time is faster than previously found travel time
                    if (totalTime < intersections[currentSegment.from].travelTime 
                            + intersections[currentSegment.from].travelTimeToDestination)  {
  //Maybe this is redundant since find_street_segments_of_intersection doesn't include one-way wrong way segments already?                      
                        if (currentSegment.oneWay == false) 
                            {
                            intersections[currentSegment.from].travelTime =  finalTravelTime;
                            intersections[currentSegment.from].reachingEdge = currentElementSegments[i];
                            
                            #ifdef VISUALIZE
                            ezgl::renderer *g = globalPointer->get_renderer();
                            drawSegment(g, currentElementSegments[i]);
                            globalPointer->flush_drawing();
                            delay(5);
                            #endif

                            waveFront.push_back(waveElement(currentSegment.from, 
                                    //(other intersection, current street segment, calculated time)
                                    currentElementSegments[i], finalTravelTime, totalTime)); 
                            
                            push_heap(waveFront.begin(), waveFront.end(), compareTravelTime());
//If the destination is found, run a function to obtain a vector of all street segments used, return that vector                            
                            if(currentSegment.from == intersect_id_end)  
                                {
                                //cout << "My time: " << intersections[currentSegment.from].travelTime << endl;
                                finalPath = obtainPath(intersections, intersect_id_start, intersect_id_end);
                                //return finalPath;
                                return totalTime;
                                } 
                            }    
                        }
                   }
                else
                    {
                    totalTime = finalTravelTime + intersections[currentSegment.to].
                            travelTimeToDestination;
                    if (totalTime < intersections[currentSegment.to].travelTime + 
                            intersections[currentSegment.to].travelTimeToDestination)  {
                        intersections[currentSegment.to].travelTime = finalTravelTime;
                        intersections[currentSegment.to].reachingEdge = currentElementSegments[i];
                        
                        #ifdef VISUALIZE
                        ezgl::renderer *g = globalPointer->get_renderer();
                        drawSegment(g, currentElementSegments[i]);
                        globalPointer->flush_drawing();
                        delay(5);
                        #endif

                        waveFront.push_back(waveElement(currentSegment.to, 
                                currentElementSegments[i], finalTravelTime, totalTime));
                        push_heap(waveFront.begin(), waveFront.end(), compareTravelTime());
 //If the destination is found, run a function to obtain a vector of all street segments used, return that vector                        
                        if(currentSegment.to == intersect_id_end) 
                            {
                           // cout << "My time: " << intersections[currentSegment.to].travelTime << endl;
                            finalPath = obtainPath(intersections, intersect_id_start, intersect_id_end);
                            //return finalPath;
                            return totalTime;
                            } 
                        }
                    }
            }
            
        }
        intersections[currentElement.intersectionID].visited = true;
    }
    //return finalPath;
    return totalTime;
    //while wavefront isn't empty, continue this algorithm until wavefront is empty or no path is visible
}
 * 
 */
