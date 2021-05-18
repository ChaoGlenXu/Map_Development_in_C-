/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "StreetsDatabaseAPI.h"
#include <OSMDatabaseAPI.h>
#include <vector>
#include "city.h"
#include "street.h"
#include <string>
#include <iostream>
#include "point.hpp"
#include <math.h>
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"
#include <utility>
#include <chrono>
#include <thread>

//#define VISUALIZE
#define NO_EDGE -1
#define NO_TIME 999999999
using namespace std;
ezgl::application *globalPointer;
float latAvg = 0;

////////////////////////////////////////////////////////////////////////////////
//////////above is glen't attemp for find_path_between_intersections////////////
//////////below is glen't attemp for find_path_between_intersections////////////
#include <list>
#define NO_EDGE -1
struct intersection_Node_Glen {//intersection node made by Glen for parallel
    LatLon location;
    vector<StreetSegmentIndex> seg;
    int intersection_id;
  
};

class Node_Glen{
    public:
        int intersection_id;
        vector <int> outgoing_edge;
        int reaching_edge;
        
        Node_Glen(){//default constructor
            ;//not sure if i should initialze value or not
            
        }
        Node_Glen(int input_intersection_id, int input_reaching_edge, int make_path_longer){
            intersection_id = input_intersection_id;
            reaching_edge = input_reaching_edge;
            outgoing_edge.push_back(make_path_longer);
        }
        
        Node_Glen(int input_intersection_id, int input_reaching_edge){
            intersection_id = input_intersection_id;
            reaching_edge = input_reaching_edge;
            //vector is initialized to be 0 as default
        }
        Node_Glen(int input_intersection_id){
            intersection_id = input_intersection_id;
        }
        
        
};

struct wave_element_Glen{
    Node_Glen* node;
    int edge_id;// ID of edge used to reach this node
    wave_element_Glen(Node_Glen* n, int id ){
        node = n;
        edge_id = id;     
    }
};

//not sure if should add #define NO_EDGE -1 yet


std::vector<StreetSegmentIndex> find_path_between_intersections_Glen(
		          const IntersectionIndex intersect_id_start, 
                  const IntersectionIndex intersect_id_end,
                  const double turn_penalty);
bool bfs_path_Glen(Node_Glen* sourceNode, int dest_ID, vector<StreetSegmentIndex>& return_vector);


//////////above is glen't attemp for find_path_between_intersections////////////
//////////above is glen't attemp for find_path_between_intersections////////////
////////////////////////////////////////////////////////////////////////////////

//Structure for intersection data during pathfinding
struct intersectionData {
  StreetSegmentIndex reachingEdge;
  double travelTime;
  double travelTimeToDestination;
  double walkTime;
  bool visited;
  
};

//Structure and constructor for wavefront elements
struct waveElement {
    IntersectionIndex intersectionID;
    StreetSegmentIndex edgeID;
    double travelTime;
    double heuristicTime;
    waveElement(int intersection, int streetsegment, double travel, double heuristic) {
        intersectionID = intersection;
        edgeID = streetsegment;
        travelTime = travel;
        heuristicTime = heuristic;
    }
    waveElement(){
        intersectionID = 0;
        edgeID = 0;
        travelTime = 0.00;
        heuristicTime = 0.00;
    }
};


//Structure function used to transform max heap into min heap
struct compareTravelTime { 
    bool operator()(waveElement a, waveElement b) {
        return a.heuristicTime > b.heuristicTime;
    }
};

bool nodeCheck(StreetSegmentIndex currentSegment, IntersectionIndex currentIntersection);
bool turnPenaltyCheck(StreetSegmentIndex currentSegment, StreetSegmentIndex previousSegment);
vector<StreetSegmentIndex> obtainPath(vector<intersectionData> intersections,
        IntersectionIndex start, IntersectionIndex end);
pair<vector<StreetSegmentIndex>, IntersectionIndex> obtainWalkingPath(vector<intersectionData>
intersections, IntersectionIndex start, IntersectionIndex end, double walking_time_limit);
void drawSegment(ezgl::renderer *g, StreetSegmentIndex id);
float xfromlon( float lon);
float yfromlat( float lat);
float lonfromx( float x);
float latfromy( float y);
void delay(int milliseconds);
// Returns the time required to travel along the path specified, in seconds.
// The path is given as a vector of street segment ids, and this function can
// assume the vector either forms a legal path or has size == 0. The travel
// time is the sum of the length/speed-limit of each street segment, plus the
// given turn_penalty (in seconds) per turn implied by the path. If there is
// no turn, then there is no penalty. Note that whenever the street id changes
// (e.g. going from Bloor Street West to Bloor Street East) we have a turn.
double compute_path_travel_time(const std::vector<StreetSegmentIndex>& path, 
        const double turn_penalty){//sharon
    
    int streetSegementNum = path.size(); //getting the total number of segments in a path
    double pathTravelTime = 0.00; //initializing summing variable 
    
    if(streetSegementNum == 0) {
        return 0.00;
    }else {//when the vector contains more than one StreetSegmentIndex, use for loop to loop through all segments
        for(int currentSegmentID = 0; currentSegmentID < streetSegementNum; currentSegmentID++){
           
            double segmentTravelTime = find_street_segment_travel_time(path[currentSegmentID]); 
     
            //check for turn penalty (if had not reach the last segment) by calling 
            //helper function, which compare street id of consecutive segments
            if (currentSegmentID!= streetSegementNum -1 && turnPenaltyCheck
                    (path[currentSegmentID], path[currentSegmentID+1]) == true) {
                pathTravelTime = pathTravelTime + segmentTravelTime + turn_penalty;
           } else {
                pathTravelTime = pathTravelTime + segmentTravelTime;
           }        
        }
        return pathTravelTime;
     }
}


// Returns a path (route) between the start intersection and the end
// intersection, if one exists. This routine should return the shortest path
// between the given intersections, where the time penalty to turn right or
// left is given by turn_penalty (in seconds). If no path exists, this routine
// returns an empty (size == 0) vector. If more than one path exists, the path
// with the shortest travel time is returned. The path is returned as a vector
// of street segment ids; traversing these street segments, in the returned
// order, would take one from the start to the end intersection.
std::vector<StreetSegmentIndex> find_path_between_intersections(const IntersectionIndex 
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
                                return finalPath;
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
                            return finalPath;
                            } 
                        }
                    }
            }
            
        }
        intersections[currentElement.intersectionID].visited = true;
    }
    return finalPath;
    //while wavefront isn't empty, continue this algorithm until wavefront is empty or no path is visible
}
//Self-note: To implement turn_penalty, maybe check for a street segment change?

//Returns the time required to "walk" along the path specified, in seconds.
//The path is given as a vector of street segment ids. The vector can be of
//size = 0, and in this case, it the function should return 0. The walking
//time is the sum of the length/<walking_speed> for each street segment, plus
//the given turn penalty, in seconds, per turn implied by the path. If there
// is no turn, then there is no penalty. As mentioned above, going from Bloor
// Street West to Bloor street East is considered a turn
double compute_path_walking_time(const std::vector<StreetSegmentIndex>& path,
        const double walking_speed, const double turn_penalty){
    
    int streetSegementNum = path.size(); //getting the total number of segments in a path
    double pathTravelTime = 0.00; //initializing summing variable 
    if(streetSegementNum == 0) {
        return 0.00;
    }else {//when the vector contains more than one StreetSegmentIndex, use for loop to loop through all segments
        for(int currentSegmentID = 0; currentSegmentID < streetSegementNum; currentSegmentID++){
           
            double segmentLength = find_street_segment_length (path[currentSegmentID]); //metres
            double segmentTravelTime = segmentLength / walking_speed; // metres / metres/second
            //check for turn penalty (if had not reach the last segment) by 
            //calling helper function, which compare street id of consecutive segments
            if (currentSegmentID!= streetSegementNum -1 && turnPenaltyCheck
                    (path[currentSegmentID], path[currentSegmentID+1]) == true) {
                pathTravelTime = pathTravelTime + segmentTravelTime + turn_penalty;
           } else {
                pathTravelTime = pathTravelTime + segmentTravelTime;
           }    
        }
 
        return pathTravelTime;
     }
}

std::pair<std::vector<StreetSegmentIndex>, std::vector<StreetSegmentIndex>> 
        find_path_with_walk_to_pick_up(const IntersectionIndex start_intersection,
                                        const IntersectionIndex end_intersection,
                                        const double turn_penalty,
                                        const double walking_speed,
                                        const double walking_time_limit){
    //declaring variables to return 
    vector <StreetSegmentIndex> initialDrivePath;
    vector<StreetSegmentIndex> initialWalkPath;
    pair <vector<StreetSegmentIndex>, vector<StreetSegmentIndex>> finalPath;
    
    double walkingTime = 0;
    double walkingSegmentCount = 0;
    
    if(walking_time_limit == 0) { //Special case: No walking time; Just return a driving path from start to finish
        initialDrivePath = find_path_between_intersections(start_intersection, end_intersection, turn_penalty);
        finalPath.second = initialDrivePath;
        return finalPath;
    }
    
    
    pair<vector<StreetSegmentIndex>, IntersectionIndex> combinedWalkingPath;
    combinedWalkingPath.second = -1;
    vector<waveElement> waveFront;
    LatLon start;
    LatLon end = getIntersectionPosition(end_intersection);    
    double totalTime;
    //Initial vector setup for pathfinding intersection data
    vector<intersectionData> intersections; 
    int intersectionCount = getNumIntersections();
    intersections.resize(intersectionCount);
    for(int i = 0; i < intersectionCount; i++) { //loop through all intersections
        intersections[i].reachingEdge = NO_EDGE;   //-1 initialize all intersection to be the starting intersection
        intersections[i].travelTime = NO_TIME; //9999 initialize all travelTime to be very big
        start = getIntersectionPosition(i);
        double heuristic = find_distance_between_two_points(make_pair(start, end));
        intersections[i].travelTimeToDestination = heuristic / walking_speed;
        intersections[i].visited = false; //initialize all intersection to be un-visited
    }
    
    //Initial setup: Get adjacent intersections to the starting intersection 
    //(excluding one-way the wrong way), calculate the travel time for each and store them in the wavefront
    intersections[start_intersection].travelTime = 0; //the time that needs to get to the first node is zero

    
    waveFront.push_back(waveElement(start_intersection, NO_EDGE, 0, 
    intersections[start_intersection].travelTimeToDestination));
    
    //Turn the current wavefront into a min heap
    make_heap(waveFront.begin(), waveFront.end(), compareTravelTime()); 
    
    while (!waveFront.empty()) { //if the wavefront is not empty, meaning there are nodes for us to check
        waveElement currentElement = waveFront.front(); //take the first element of the wavefront
        
      //Take the element with smallest travel time and remove it from the heap
        pop_heap(waveFront.begin(), waveFront.end(), compareTravelTime()); 
        waveFront.pop_back(); 
        
         
        if (intersections[currentElement.intersectionID].visited == false) {
            vector<int> currentElementSegments = //Find outward edges of current intersection
            find_street_segments_of_intersection(currentElement.intersectionID); 
            
            //Analyze outward edges one at a time, finding travel times to new nodes
            int currentSegmentNum = currentElementSegments.size();
            for (int i = 0; i < currentSegmentNum; i++) {
                InfoStreetSegment currentSegment = getInfoStreetSegment(currentElementSegments[i]);
                double finalTravelTime;
                double newLength = find_street_segment_length(currentElementSegments[i]);
                
                double newTravelTime = newLength / walking_speed;
                
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
                            intersections[currentSegment.from].travelTime =  finalTravelTime;
                            intersections[currentSegment.from].reachingEdge = currentElementSegments[i];
                            //(other intersection, current street segment, calculated time)
                            waveFront.push_back(waveElement(currentSegment.from,
                                    currentElementSegments[i], finalTravelTime, totalTime)); 
                            push_heap(waveFront.begin(), waveFront.end(), compareTravelTime());
 //If the destination is found, run a function to obtain a vector of all street segments used, return that vector
                            if(currentSegment.from == end_intersection)  
                                {
                            combinedWalkingPath = obtainWalkingPath(intersections,
                                    start_intersection, end_intersection, walking_time_limit);
                                }     
                        }
                   }
                else
                    {
                    totalTime = finalTravelTime + intersections[currentSegment.to].travelTimeToDestination;
                    if (totalTime < intersections[currentSegment.to].travelTime 
                            + intersections[currentSegment.to].travelTimeToDestination)  {
                        intersections[currentSegment.to].travelTime = finalTravelTime;
                        intersections[currentSegment.to].reachingEdge = currentElementSegments[i];
                        waveFront.push_back(waveElement(currentSegment.to, 
                                currentElementSegments[i], finalTravelTime, totalTime));
                        push_heap(waveFront.begin(), waveFront.end(), compareTravelTime());
//If the destination is found, run a function to obtain a vector of all street segments used, return that vector
                        if(currentSegment.to == end_intersection)  
                            {
                            combinedWalkingPath = obtainWalkingPath(intersections, 
                                    start_intersection, end_intersection, walking_time_limit);
                            } 
                        }
                    }
            }
            
        }
        intersections[currentElement.intersectionID].visited = true;
    }
    // No walking path found or entire path is walkable
    if(combinedWalkingPath.second == -1 || combinedWalkingPath.second == end_intersection) { 
        initialDrivePath = find_path_between_intersections(start_intersection, end_intersection, turn_penalty);
    }

    else {
       // cout << "Pick up at: " << combinedWalkingPath.second << endl;
       initialDrivePath = find_path_between_intersections(combinedWalkingPath.second, end_intersection, turn_penalty);
    }
    finalPath.first = combinedWalkingPath.first;
    finalPath.second = initialDrivePath;
    return finalPath;

    
}


//for a oneway segment, check if 
bool nodeCheck(StreetSegmentIndex currentSegment, IntersectionIndex currentIntersection){
    InfoStreetSegment currInfo = getInfoStreetSegment(currentSegment);
    if (currInfo.oneWay == false ||  ( currInfo.oneWay == false && currentIntersection == currInfo.from)) {
        return true;
    } else {
        return false;
    }
   
}

bool turnPenaltyCheck(StreetSegmentIndex currentSegment, StreetSegmentIndex nextSegment){
    if(currentSegment == NO_EDGE || nextSegment == NO_EDGE) {
        return false;
    }
    
    InfoStreetSegment currentInfo = getInfoStreetSegment(currentSegment);
    InfoStreetSegment previousInfo = getInfoStreetSegment(nextSegment);
    
    
    if (currentInfo.streetID != previousInfo.streetID) {
        return true;
    } else {
        return false;
    }
    
} 


//Returns a vector of street segments that makes up the path
vector<StreetSegmentIndex> obtainPath(vector<intersectionData> intersections,
        IntersectionIndex start, IntersectionIndex end) {
    vector<StreetSegmentIndex> allSegments;
    IntersectionIndex currentIntersection = end;

    //Get previous intersection
    StreetSegmentIndex currentEdge = intersections[currentIntersection].reachingEdge;
    while (currentEdge != NO_EDGE)
        {

        allSegments.push_back(currentEdge);
        InfoStreetSegment currentEdgeInfo = getInfoStreetSegment(currentEdge);
        //Determines which side of the segment the current intersection is at, 
        //then sets the previous intersection accordingly, along with the new reachingEdge
        if (currentIntersection == currentEdgeInfo.to) {
            currentIntersection =  currentEdgeInfo.from;
            currentEdge = intersections[currentIntersection].reachingEdge;
            
            }
        else {
            currentIntersection = currentEdgeInfo.to;
            currentEdge = intersections[currentIntersection].reachingEdge;
            }
        }
    
        
        reverse(allSegments.begin(), allSegments.end());
        return allSegments;

}

pair<vector<StreetSegmentIndex>, IntersectionIndex> obtainWalkingPath(
vector<intersectionData> intersections, IntersectionIndex start, IntersectionIndex end,
        double walking_time_limit) {
    vector<StreetSegmentIndex> allSegments;
    IntersectionIndex currentIntersection = end;
    IntersectionIndex endofWalk = -1;
    bool firstIntersection = false;
    //Get previous intersection
    StreetSegmentIndex currentEdge = intersections[currentIntersection].reachingEdge;
    while (currentEdge != NO_EDGE)
        {
        //cout << "Travel Time: " << intersections[currentIntersection].travelTime <<  " / " << walking_time_limit << endl;
        if(intersections[currentIntersection].travelTime < walking_time_limit) {
            if(firstIntersection == false) {
                endofWalk = currentIntersection;
                firstIntersection = true;
            }
        allSegments.push_back(currentEdge);
        }
        InfoStreetSegment currentEdgeInfo = getInfoStreetSegment(currentEdge);
        //Determines which side of the segment the current intersection is at, 
        //then sets the previous intersection accordingly, along with the new reachingEdge
        if (currentIntersection == currentEdgeInfo.to) {
            currentIntersection =  currentEdgeInfo.from;
            currentEdge = intersections[currentIntersection].reachingEdge;
            
            }
        else {
            currentIntersection = currentEdgeInfo.to;
            currentEdge = intersections[currentIntersection].reachingEdge;
            }
        }
    
        
        reverse(allSegments.begin(), allSegments.end());
        
        return make_pair(allSegments, endofWalk);

}


//////////below is glen't attemp for find_path_between_intersections////////////
std::vector<StreetSegmentIndex> find_path_between_intersections_Glen(
		          const IntersectionIndex intersect_id_start, 
                  const IntersectionIndex intersect_id_end,
                  const double turn_penalty){
    Node_Glen* sourceNode = new Node_Glen(intersect_id_start, NO_EDGE );
    vector<StreetSegmentIndex> return_vector ;//if there's no path, then the  
    bool found = bfs_path_Glen(sourceNode, intersect_id_end, return_vector);
    return return_vector;
}

bool bfs_path_Glen(Node_Glen* sourceNode, int dest_ID, vector<StreetSegmentIndex>& return_vector){

    list<wave_element_Glen> wave_front;
    wave_front.push_back(wave_element_Glen(sourceNode, NO_EDGE)); // no edge to reach the source node
    
    while(wave_front.size() > 0){
        wave_element_Glen current = wave_front.front();
        wave_front.pop_front(); //remove node from wave_front
        current.node->reaching_edge = current.edge_id;//readching _edge is the id that used to reach the current node
        
        if(current.node->intersection_id == dest_ID){
            return_vector = current.node->outgoing_edge;
            return true;
        }
        
        //create the temporary segment here for extracting some data in the for loop 
        InfoStreetSegment temp_seg;
        
        
        //later i could use index to make it faster
        //for(vector<int>::iterator it = current.node->outgoing_edge.begin(); it !=  current.node->outgoing_edge.end(); it++ ){
        int total_number_of_edge = getIntersectionStreetSegmentCount(current.node->intersection_id);
        int temporary_seg_index;
        for(int i = 0; i < total_number_of_edge; i++){    //need to not include to_edge
            temporary_seg_index = getIntersectionStreetSegment( current.node->intersection_id , i );
            temp_seg = getInfoStreetSegment(temporary_seg_index);
            if(temporary_seg_index == current.node->reaching_edge){
                ;//we don't do anything here
            }else{ //we do things here
                
                
                //check if it is oneway or not, if is, then check if path is valid
                if(temp_seg.oneWay == true){ // it is oneway
                    if(current.node->intersection_id == temp_seg.from){//path is valid
                        Node_Glen* to_node = new Node_Glen(temp_seg.to, temporary_seg_index );
                        wave_front.push_back(wave_element_Glen(to_node, temporary_seg_index ));
                    }else{//path is not valid
                        ;
                    }
                        
                }else{// it is bidirectional
                    if(current.node->intersection_id == temp_seg.from){//if current is from, then the to_node is temp_seg.to
                        Node_Glen* to_node = new Node_Glen(temp_seg.to, temporary_seg_index );
                        wave_front.push_back(wave_element_Glen(to_node, temporary_seg_index ));
                    }else{//if current is to, then the to_node is temp_seg.from
                        Node_Glen* to_node = new Node_Glen(temp_seg.from, temporary_seg_index );
                        wave_front.push_back(wave_element_Glen(to_node, temporary_seg_index ));
                    }
                }
                //the intersection_id may not be needed, for now just implement it, think about it later
//                Node_Glen* to_node = new Node_Glen(, temporary_seg_index );
//            
//                wave_front.push_back(wave_element_Glen(to_node, temporary_seg_index ));
                
                
            }
        }
    }
    
    return(false);
}


//////////above is glen't attemp for find_path_between_intersections////////////

void drawSegment(ezgl::renderer *g, StreetSegmentIndex id) {
       g->set_color(ezgl::BLUE);
         //if the segment has zero curve points
        double x1;
        double y1;
        double x2;
        double y2;
        double xtotal = 0;
        double ytotal = 0;
        int point_Count = getInfoStreetSegment(id).curvePointCount; 
        
 //if there is no curve point, call the function of distance between two points directly        
        if(getInfoStreetSegment(id).curvePointCount == 0) { 
            InfoStreetSegment currentSegment = getInfoStreetSegment(id);
            LatLon point1 = getIntersectionPosition(currentSegment.from);
            LatLon point2 = getIntersectionPosition(currentSegment.to);
            x1 = xfromlon(point1.lon());
            y1 = yfromlat(point1.lat());
            x2 = xfromlon(point2.lon());
            y2 = yfromlat(point2.lat());
            
            
                g->draw_line({x1, y1}, {x2, y2});

               
                
        } else { //if there are curve points//my method: make it three segments
            LatLon point1 = getIntersectionPosition(getInfoStreetSegment(id).from);
            x1 = xfromlon(point1.lon());
            y1 = yfromlat(point1.lat());
            LatLon point2 = getStreetSegmentCurvePoint(0, id);
            x2 = xfromlon(point2.lon());
            y2 = yfromlat(point2.lat());
            xtotal = xtotal + x1 + x2;
            ytotal = ytotal + y1 + y2;
            g->draw_line({x1, y1}, {x2, y2});

            for (int k=0; k< (getInfoStreetSegment(id).curvePointCount)-1; k++){
                point1 = getStreetSegmentCurvePoint(k, id); 
                point2 = getStreetSegmentCurvePoint(k+1, id);
                x1 = xfromlon(point1.lon());
                y1 = yfromlat(point1.lat());
                x2 = xfromlon(point2.lon());
                y2 = yfromlat(point2.lat());
                xtotal = xtotal + x2;
                ytotal = ytotal + y2;
                g->draw_line({x1, y1}, {x2, y2});
            }
            point1 = getStreetSegmentCurvePoint(point_Count-1, id); 
            point2 = getIntersectionPosition(getInfoStreetSegment(id).to);
            x1 = xfromlon(point1.lon());
            y1 = yfromlat(point1.lat());             
            x2 = xfromlon(point2.lon());
            y2 = yfromlat(point2.lat());
            xtotal = xtotal + x2;
            ytotal = ytotal + y2;
            g->draw_line({x1, y1}, {x2, y2}); 
        }
}
        
        


float xfromlon( float lon){
     float x_value = lon * cos(latAvg * DEGREE_TO_RADIAN);
     return x_value;
}

float yfromlat( float lat){
     float y_value = lat;
     return y_value;
}

float lonfromx( float x){
    float lon = x/cos(latAvg * DEGREE_TO_RADIAN);
    return lon;
}

float latfromy( float y){
    return y;
}

void delay(int milliseconds) {
    std::chrono::milliseconds duration (milliseconds);
    std::this_thread::sleep_for(duration);
}