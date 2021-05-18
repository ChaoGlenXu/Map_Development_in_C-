/*
 * Copyright 2020 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated
 * documentation files (the "Software") in course work at the University
 * of Toronto, or for personal use. Other uses are prohibited, in
 * particular the distribution of the Software either publicly or to third
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <algorithm>
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include <math.h>
#include <OSMDatabaseAPI.h>
#include <vector>
#include <map> 
#include <cctype>
#include <boost/algorithm/string.hpp>
#include "city.h"
#include "street.h"
#include <string>

using namespace std;

string lowercase_no_whitespace(string input);


vector<vector<int>> intersectionStreetSegments;
map<const OSMWay*, OSMID> wayAndID;
map<const OSMNode*, OSMID> nodeAndID;
city theCity;
multimap<string, int> street_names_for_prefix;

bool load_map(std::string fn /*map_path*/) { //take in desired location
    bool load_successful = false;
    string streetToOSM = "streets.bin";
    
    load_successful = loadStreetsDatabaseBIN(fn);//Indicates whether the map has loaded
                                  //successfully
    if (load_successful == true) {
        auto streetPos = fn.find(streetToOSM); 
        string osmString = fn;
        osmString.erase(streetPos, streetToOSM.length());
        loadOSMDatabaseBIN(osmString +  "osm.bin");
        
        
    //Finds OSMWay pointer of given way_id
    for (int i = 0; i < getNumberOfWays(); i ++) {
        const OSMWay* way = getWayByIndex(i);
        wayAndID.insert(make_pair(way, way->id()));
    }
    
    for (int i = 0; i < getNumberOfNodes(); i++) 
    {
        const OSMNode* node = getNodeByIndex(i);
        nodeAndID.insert(make_pair(node, node->id()));
    }
        
        intersectionStreetSegments.resize(getNumIntersections());
        //resizing the size of the "allStreets" vector, for memory efficiency
        theCity.allStreets.resize(getNumStreets()); 
        
        //loop through all the street segments, starting from one
        for (int ssID = 0; ssID < getNumStreetSegments(); ssID++) { 
//        //ssID = streetSegmentID
//        //the first street segment has the ssID of 0
            
            //get the struct infoStreetSegment using the ssID
            InfoStreetSegment info_ss = getInfoStreetSegment(ssID); 

        //Checking for potential duplicates in both vectors: intersections and street segments.
        //Any duplicate is not added to the vector.
            auto duplicate = find(theCity.allStreets[info_ss.streetID].allIntersections.begin(), 
                    theCity.allStreets[info_ss.streetID].allIntersections.end(), info_ss.from);
            if (duplicate == theCity.allStreets[info_ss.streetID].allIntersections.end())
               {
               theCity.allStreets[info_ss.streetID].allIntersections.push_back(info_ss.from);
               }
           duplicate = find(theCity.allStreets[info_ss.streetID].allIntersections.begin(),
                   theCity.allStreets[info_ss.streetID].allIntersections.end(), info_ss.to);
           if (duplicate == theCity.allStreets[info_ss.streetID].allIntersections.end())
               {   
                theCity.allStreets[info_ss.streetID].allIntersections.push_back(info_ss.to);
               }

            duplicate = find(theCity.allStreets[info_ss.streetID].allStreetSegments.begin(),
                    theCity.allStreets[info_ss.streetID].allStreetSegments.end(), ssID);
            if (duplicate == theCity.allStreets[info_ss.streetID].allStreetSegments.end()) {
            theCity.allStreets[info_ss.streetID].allStreetSegments.push_back(ssID);
            }
        }
    //A separate loop for a nested vector containing street segments of an intersection
    for(int intersection = 0; intersection < getNumIntersections(); intersection++) {
        for (int i = 0; i < getIntersectionStreetSegmentCount(intersection); i++) {
            int segment_id = getIntersectionStreetSegment(intersection,i);
            intersectionStreetSegments[intersection].push_back(segment_id);
            }
        }
    //a separate loop for a database for street name with binary tree 
        string street_name;
        for(int i = 0; i < getNumStreets(); i++){
            //get the street name by it's id
            //remove whitespace & convert to lowercase  
            street_name = lowercase_no_whitespace(getStreetName(i));
            street_names_for_prefix.insert(std::make_pair(street_name, i));//insert street name

        }

    }


    return load_successful;
}

void close_map() {
    //Clean-up your map related data structures here
    closeStreetDatabase();
    closeOSMDatabase();
    
    street_names_for_prefix.clear();
}


//Takes a pair of Lat & Lon values, and converts into distance in meters
double find_distance_between_two_points(std::pair<LatLon, LatLon> points) { // Andrew
    double latAvg =  (((points.first).lat())*DEGREE_TO_RADIAN + ((points.second).lat())*DEGREE_TO_RADIAN) / 2;
    double pointOneX = (points.first).lon()*DEGREE_TO_RADIAN * cos(latAvg);
    double pointOneY = (points.first).lat()*DEGREE_TO_RADIAN;
    double pointTwoX = (points.second).lon()*DEGREE_TO_RADIAN * cos(latAvg);
    double pointTwoY = (points.second).lat()*DEGREE_TO_RADIAN;
    
    return EARTH_RADIUS_METERS * sqrt( pow( (pointTwoY - pointOneY) ,2) + pow ( (pointTwoX - pointOneX) , 2));
}

double find_street_segment_length(int street_segment_id) { // Sharon
 
    int point_Count = 0; //create variable to store the number of curve points of a road
    point_Count = getInfoStreetSegment(street_segment_id).curvePointCount; //obtain the data
    double segment_length_1 = 0;
    double segment_length_2 = 0;
    double segment_length_3 = 0;
    
    //if the segment has zero curve points
    if(getInfoStreetSegment(street_segment_id).curvePointCount == 0) { //if there is no curve point, call the function of distance between two points directly 
        return find_distance_between_two_points(make_pair(getIntersectionPosition
                (getInfoStreetSegment(street_segment_id).from), getIntersectionPosition
                (getInfoStreetSegment(street_segment_id).to)));
    } else { //if there are curve points
        
    segment_length_1 = find_distance_between_two_points(make_pair(getIntersectionPosition
            (getInfoStreetSegment(street_segment_id).from), getStreetSegmentCurvePoint(0, street_segment_id)));
    for (int i=0; i< (getInfoStreetSegment(street_segment_id).curvePointCount)-1; i++){
        segment_length_2 += find_distance_between_two_points(make_pair(getStreetSegmentCurvePoint
                (i, street_segment_id), getStreetSegmentCurvePoint(i+1, street_segment_id)));
    }
    segment_length_3 = find_distance_between_two_points(make_pair(getStreetSegmentCurvePoint
            (point_Count-1, street_segment_id),getIntersectionPosition(getInfoStreetSegment(street_segment_id).to)));
    
     return segment_length_1 + segment_length_2 + segment_length_3; // return the sum directly 
    }
}

//Returns the travel time to drive a street segment in seconds 
//(time = distance/speed_limit)
double find_street_segment_travel_time(int street_segment_id) { // Glen 
    double distance = find_street_segment_length(street_segment_id);
    double speedLimit = (getInfoStreetSegment(street_segment_id).speedLimit)*(1.0/3.6);
    return distance / speedLimit;
} 

    
//Use distance formula, loop through intersection index, return lowest distance ID?
//Finds the closest intersection to a given pair of Lat & Lon Values
int find_closest_intersection(LatLon my_position) { // Andrew
    //Use distance formula, loop through intersection index, return lowest distance ID
    std::pair<LatLon, LatLon> distanceCheck;
    distanceCheck.first = my_position;
    distanceCheck.second = getIntersectionPosition(0);
    double dist = find_distance_between_two_points(distanceCheck);
    int id = 0;
    //Set up a pair with position and intersection ID 0, get initial distance to prepare for loop
    
    for (int i = 1; i < getNumIntersections(); i++)
        {
        distanceCheck.second = getIntersectionPosition(i);
        double newdist = find_distance_between_two_points(distanceCheck);
        if (newdist < dist)
            {
            dist = newdist;
            id = i;
            //If new distance is shorter than existing distance, set ID to the intersection resulting in shorter distance
            }
        }
    return id; //Return ID with shortest distance
    
    //**O(n) complexity. Can this be optimized?
}

//Returns the street segments for the given intersection
std::vector<int> find_street_segments_of_intersection(int intersection_id) { // S
    return intersectionStreetSegments[intersection_id]; //directly return the result form load_map data structure
}


//Returns the street names at the given intersection (includes duplicate street 
//names in returned vector)
std::vector<std::string> find_street_names_of_intersection(int intersection_id) { // Glen
    vector<std::string> street_name_collection;//return value
    //get the total number for the segments for each id
    int count_intersection_segment = getIntersectionStreetSegmentCount(intersection_id);
    for(int i = 0; i < count_intersection_segment; i++ ){//loop through the segments
        int street_segment_id = getIntersectionStreetSegment( intersection_id , i );
        //get the street segment struct
        InfoStreetSegment street_segment = getInfoStreetSegment( street_segment_id );
        string street_name = getStreetName(street_segment.streetID );
        //put the street name in to collection
        street_name_collection.push_back(street_name);
    }
    return street_name_collection;
}

//Returns true if you can get from intersection_ids.first to intersection_ids.second using a single
//street segment (hint: check for 1-way streets too)
bool are_directly_connected(std::pair<int, int> intersection_ids) { // Andrew
    vector<int> firstSegments = find_street_segments_of_intersection(intersection_ids.first);
    //Collect segments of each intersection
    vector<int> secondSegments = find_street_segments_of_intersection(intersection_ids.second); 
    if (intersection_ids.first == intersection_ids.second)
        {
        return true;
        }
    
    for (int i = 0; i < firstSegments.size(); i++)
        {
        auto connection = std::find(secondSegments.begin(), secondSegments.end(), 
                firstSegments[i]); //Cycle through all firstSegments and see if it exists in secondSegments
        if (connection != secondSegments.end())
            {
            return true;
            }
        }
return false;
}

//Returns all intersections reachable by traveling down one street segment
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections
std::vector<int> find_adjacent_intersections(int intersection_id) { // S
    vector <int> intersection_list; //create a vector to store all the adjacent intersectionID
    
    //get the number of street segments of the street
    int count = getIntersectionStreetSegmentCount(intersection_id); 
    
    for (int i=0; i < count; i++){//loop through all the street segments
        int segment_Num = getIntersectionStreetSegment(intersection_id, i);
        bool ID_existed = false;
        
        //case 1: when street is limited to oneWay AND starting point is the given intersection, save TO
        if(getInfoStreetSegment(segment_Num).oneWay == true && getInfoStreetSegment
                (segment_Num).from == intersection_id){
            for(int j = 0;j<intersection_list.size();j++)
            {
                if(intersection_list[j] == getInfoStreetSegment(segment_Num).to){
                    ID_existed = true;
                }
            }
            if (!ID_existed){//if the intersectionID already existed in the vector, do not store
            intersection_list.push_back(getInfoStreetSegment(segment_Num).to);
            }
        }
        //case 2: when street is NOT limited to oneWay AND the FROM is the given intersection, save TO
        if(getInfoStreetSegment(segment_Num).oneWay == false && getInfoStreetSegment
                (segment_Num).from == intersection_id){
            for(int j = 0;j<intersection_list.size();j++)
            {
                if(intersection_list[j] == getInfoStreetSegment(segment_Num).to){
                    ID_existed = true;
                }
            }
            if (!ID_existed){//if the intersectionID already existed in the vector, do not store
                intersection_list.push_back(getInfoStreetSegment(segment_Num).to);

            }
        }
         //case 3: when street is NOT limited to oneWay AND the to is the given intersection, save FROM
        if(getInfoStreetSegment(segment_Num).oneWay == false && getInfoStreetSegment
                (segment_Num).to == intersection_id){
            for(int j = 0;j<intersection_list.size();j++){
                if(intersection_list[j] == getInfoStreetSegment(segment_Num).from){
                    ID_existed = true;
                }
            }
            if (!ID_existed){//if the intersectionID already existed in the vector, do not store
            intersection_list.push_back(getInfoStreetSegment(segment_Num).from);
            }
        }
    }
    return intersection_list;
}


//Returns all street segments for the given street
std::vector<int> find_street_segments_of_street(int street_id) { // Glen
    return theCity.allStreets[street_id].allStreetSegments;
}

//Returns all intersections along the a given street

std::vector<int> find_intersections_of_street(int street_id) { // A
     return theCity.allStreets[street_id].allIntersections;
}


//Return all intersection ids for two intersecting streets
//This function will typically return one intersection id.
std::vector<int> find_intersections_of_two_streets(std::pair<int, int> street_ids) { // Sharon
    
    //Creates vectors containing intersections of each street
    vector <int> doubleIntersections;
    vector<int> firstIntersections = find_intersections_of_street(street_ids.first);
    vector<int> secondIntersections = find_intersections_of_street(street_ids.second);

    //Loops through all intersections in one vector, searching the other vector for that same intersection
    for (int i = 0; i < firstIntersections.size(); i++) {
        auto twoIntersection = find(secondIntersections.begin(), secondIntersections.end(), firstIntersections[i]);
        if (twoIntersection != secondIntersections.end()) {
            doubleIntersections.push_back(firstIntersections[i]);
            }
        }
    
    return doubleIntersections; //return the nested vector
}



//Returns all street ids corresponding to street names that start with the given prefix
//The function should be case-insensitive to the street prefix. You should ignore spaces.
//For example, both "bloor " and "BloOrst" are prefixes to "Bloor Street East".
//If no street names match the given prefix, this routine returns an empty (length 0)
//vector.
//You can choose what to return if the street prefix passed in is an empty (length 0)
//string, but your program must not crash if street_prefix is a length 0 string.
std::vector<int> find_street_ids_from_partial_street_name(std::string street_prefix) { // Glen
    string streetname_lowercase = lowercase_no_whitespace(street_prefix);
    vector<int> street_prefix_collection;//a collection of street ids
    multimap<string, int>::iterator it, lowerbound, upperbound;
    lowerbound = street_names_for_prefix.lower_bound(streetname_lowercase);
    bool exists = false;
    //use it to loop through all the string containg the prefix
    int string_length = streetname_lowercase.length();
    char last_char = streetname_lowercase[ string_length - 1];
    char last_char_add_1 = last_char + 1; 
    string upperBound = streetname_lowercase;
    upperBound[string_length -1] = last_char_add_1;
    upperbound = street_names_for_prefix.upper_bound(upperBound);
    for( it = lowerbound ; it != upperbound; it++){
        exists = false;
        for(int i = 0; i<street_prefix_collection.size(); i++)
        {
            if(getStreetName((*it).second) == getStreetName(street_prefix_collection[i]))
            {
                exists = true;
            }
        }// based on the 
        if(!exists)
        street_prefix_collection.push_back((*it).second);  
    }
    return street_prefix_collection;
}

//Returns the area of the given closed feature in square meters
double find_feature_area(int feature_id) { // Andrew
  int pointCount = getFeaturePointCount(feature_id);  //Variable for code clarity
 if (getFeaturePoint(0, feature_id).lon() != getFeaturePoint(pointCount - 1, feature_id).lon() 
         || (getFeaturePoint(0, feature_id).lat() != getFeaturePoint(pointCount - 1, feature_id).lat())) {
    return 0; //If latitude or longitude values don't match for beginning and end of feature, the polygon is not closed
    }
 else
    {
    vector<double> pointsX;
    vector<double> pointsY;
    double latTotal = 0;
    double latAvg = 0;
    LatLon featurePoint;
    
    //Two for-loops to obtain x and y values for each feature point
    for (int i = 0; i < pointCount; i++)
        {
        featurePoint = getFeaturePoint(i, feature_id);
        pointsY.push_back(featurePoint.lat()*DEGREE_TO_RADIAN);
        latTotal = latTotal + featurePoint.lat()*DEGREE_TO_RADIAN;
        }
   
    latAvg = latTotal / pointCount;
    
    for (int i = 0; i < pointCount; i++)
        {
        featurePoint = getFeaturePoint(i, feature_id);
        pointsX.push_back(featurePoint.lon()*DEGREE_TO_RADIAN * cos(latAvg));
        }
    
    double area = 0;
    double sum1 = 0;
    double sum2 = 0;
    //Shoelace algorithm used to calculate area
    for (int i = 0; i < pointCount - 1; i++)
        {
        sum1 = sum1 + (pointsX[i] * pointsY[i+1]);
        sum2 = sum2 + (pointsX[i+1] * pointsY[i]);
        }
    area = sum1 - sum2 + (pointsX[pointsX.size() - 1] * pointsY[0]) - (pointsX[0] 
            * pointsY[pointsY.size() - 1]); //Corner cases of shoelace
    area = abs((area / 2)* EARTH_RADIUS_METERS * EARTH_RADIUS_METERS);
    return area;
    }
}

//Returns the length of the OSMWay that has the given OSMID, in meters.
//To implement this function you will have to  access the OSMDatabaseAPI.h
//functions.
double find_way_length(OSMID way_id) { // Andrew
//    const OSMNode* node;
//    double distance_sum = 0;
//    
//    
//    bool closedWay = isClosedWay(way);
//    
//    //Obtains OSMIDs of nodes in the way member
//    vector<OSMID> nodeList_ID;
//    nodeList_ID = getWayMembers(way);
//    int listSize = nodeList_ID.size();
//    vector<const OSMNode*> nodeList_OSM(listSize);
//    int foundPosition;
//    
//    
//    // Fills up a vector of duplicate nodes with initial position and duplicate 
//    //position, as duplicates are not covered by the following for loop
//    vector<int> listOfDuplicates;
//    for (int i = 0; i < nodeList_ID.size(); i++) {
//        auto duplicate = find(nodeList_ID.begin() + i + 1, nodeList_ID.end(), nodeList_ID[i]);
//        if (duplicate != nodeList_ID.end()) {
//           int duplicatePosition = distance(nodeList_ID.begin(), duplicate);
//           listOfDuplicates.push_back(i);
//           listOfDuplicates.push_back(duplicatePosition);
//        }
//    }
//    
//    // Uses OSMIDs of nodes and places them at appropriate indices to maintain correct order
//    for (int i = 0; i < getNumberOfNodes(); i++) {
//        node = getNodeByIndex(i);
//        auto found = find(nodeList_ID.begin(), nodeList_ID.end(), node->id());
//        if (found != nodeList_ID.end()) 
//            {
//            foundPosition = distance(nodeList_ID.begin(), found);
//            nodeList_OSM[foundPosition] = node;
//            }
//    }
//    
//    // If the way is closed, then the last node will be set to the first node
//    if(closedWay == true) {
//        nodeList_OSM.erase(nodeList_OSM.end() - 1);
//        nodeList_OSM.push_back(nodeList_OSM[0]);
//    }
//   
//    // Refills duplicates with their appropriate node before distance calculation
//    for (int i = 0; i < listOfDuplicates.size();) {
//        nodeList_OSM[listOfDuplicates[i+1]] = nodeList_OSM[listOfDuplicates[i]];
//        i = i + 2;
//    }
//    
//    //Converts OSM ids of nodes to LatLon coordinates, and uses the 
//    //find_distance_between_two_points for each node to obtain total distance.
//    vector<LatLon> nodeList_LatLon;
//    for (int i = 0; i < nodeList_OSM.size(); i++) {
//        LatLon OSMLatLon = getNodeCoords(nodeList_OSM[i]);
//        nodeList_LatLon.push_back(OSMLatLon);
//    }
//  
//    
//    for (int i = 0; i < nodeList_LatLon.size() - 1; i++) {
//        pair<LatLon,LatLon> points;
//        points.first = nodeList_LatLon[i];
//        points.second = nodeList_LatLon[i+1];
//        distance_sum = distance_sum + find_distance_between_two_points(points);
//    }
//   
//  
//    return distance_sum;   
}


string lowercase_no_whitespace(string input){//Glen
    string name = input;
    //remove white space in the string
    name.erase(remove_if(name.begin(), name.end(), ::isspace),name.end()) ;
    //convert the string all into lowercase
    boost::to_lower(name);
    return name;
}