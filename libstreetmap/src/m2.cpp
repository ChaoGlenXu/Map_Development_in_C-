#include "m2.h"
#include "m3.h"
#include "m3.cpp"
#include "m1.h"
#include "StreetsDatabaseAPI.h"
#include <OSMDatabaseAPI.h>
#include <vector>
#include "city.h"
#include "street.h"
#include <string>
#include <iostream>
#include <sstream>
#include "point.hpp"
#include <math.h>
#include "ezgl/application.hpp"
#include "ezgl/graphics.hpp"

using namespace std;


bool click_2_intersections_bottom_pressed = false;
string four_street_name;
string walk_drive_four_street_name;

void draw_main_canvas(ezgl::renderer *g);
void find_button(GtkWidget */*widget*/, ezgl::application *application);
float x_from_lon( float lon);//helpper function//glen
float y_from_lat( float lat);//helpper function//glen
void act_on_mouse_click(ezgl::application* app,   GdkEventButton* event, double x, double y);
float lon_from_x( float x);//helpper function//glen & sharon
float lat_from_y( float y);//helpper function//glen & sharon

double streetWidthCheck(ezgl::renderer *g, double base);
void initial_setup(ezgl::application *application, bool /*new_window*/);
void test_FIND_button(GtkWidget */*Widget*/, ezgl::application *application);
void test_night_mode_button(GtkWidget */*Widget*/, ezgl::application *application);
void test_day_mode_button(GtkWidget */*Widget*/, ezgl::application *application);
void pathing_test(GtkWidget */*Widget*/, ezgl::application *application);
void findIntByID(GtkWidget */*Widget*/, ezgl::application *application);

bool highwayCheck(InfoStreetSegment segment);
bool biggestRoadCheck(InfoStreetSegment segment);
void drawPOI_Name(ezgl::renderer *g);
bool oneWayCheck(InfoStreetSegment segment);
void act_on_mouse_move(ezgl::application */*application*/, GdkEventButton */*event*/, double x, double y);
void act_on_key_press(ezgl::application *application, GdkEventKey */*event*/, char *key_name);

void learn_UI_button(GtkWidget */*Widget*/, ezgl::application *application);
void click_2_intersections_bottom(GtkWidget */*Widget*/, ezgl::application *application);
void find_path_by_typing_into_search_bar_button(GtkWidget */*Widget*/, ezgl::application *application);
void find_walk_drive_path_by_typing_into_search_bar_button(GtkWidget */*Widget*/, ezgl::application *application);

ezgl::color if_background_darker();
bool open_night_mode = false;//global variable for turn light on or off

 vector <int> fromX;
 vector <int> fromY;
 vector <int> toX;
 vector <int> toY;
 vector <double> allAngle;
 vector <string> street_Names;

 //Intersection data structure (revamped for pathfinding by adding reachingEdge and travelTime)
struct intersection_data {
  LatLon position;
  std::string name;
  bool highlight = false;
};


std::vector<intersection_data> intersections;
ezgl::color background(246, 242, 242, 255);


string existing_street_name;//load correct street name
int flag_for_found_intersection_by_find_bottom;

int  mouse_clicked_1st;
int  mouse_clicked_2nd;
bool click_odd_times = true;
int id_1st_intersection_by_search;
int id_2nd_intersection_by_search;

    // take in the walking speed and walking time limit for global
    int global_walking_speed;
    int global_walking_time_limit;

void draw_map(){
 fromX.clear();
 fromY.clear();
 toX.clear();
 toY.clear();
 allAngle.clear();
 street_Names.clear();
 
  ezgl::application::settings settings;

  // Path to the "main.ui" file that contains an XML description of the UI.
  settings.main_ui_resource = "libstreetmap/resources/main.ui";

  // Note: the "main.ui" file has a GtkWindow called "MainWindow".
  settings.window_identifier = "MainWindow";

  // Note: the "main.ui" file has a GtkDrawingArea called "MainCanvas".
  settings.canvas_identifier = "MainCanvas";
  
  // Create our EZGL application.
  ezgl::application application(settings);
  
   //ezgl::rectangle initial_world({0, 0}, {1000, 1000});
   
  //Loading Intersection Data 
  double max_lat = getIntersectionPosition(0).lat(); 
  double min_lat = max_lat;
  double max_lon = getIntersectionPosition(0).lon();
  double min_lon = max_lon;
  
  // Fills the intersection vector with names and positions corresponding to their ID
  intersections.resize(getNumIntersections());
  for (int id = 0; id < getNumIntersections(); ++id) {
    intersections[id].position = getIntersectionPosition(id);   
    intersections[id].name = getIntersectionName(id);
    //Loop to find max/min values of latitude/longitude
    max_lat = max(max_lat, intersections[id].position.lat());
    min_lat = min(min_lat, intersections[id].position.lat());
    max_lon = max(max_lon, intersections[id].position.lon());
    min_lon = min(min_lon, intersections[id].position.lon());
  }
   
  latAvg =  ( max_lat + min_lat ) / 2;//we need it for func: x_from_lon
  
for (int countStreetSegment = 0 ; countStreetSegment < getNumStreetSegments() ; countStreetSegment++){
    
    InfoStreetSegment currentSegmentInformation = getInfoStreetSegment(countStreetSegment);
    double from_Lat = getIntersectionPosition(currentSegmentInformation. from ) .lat();
    double from_Lon = getIntersectionPosition(currentSegmentInformation. from ) .lon();
    double to_Lat = getIntersectionPosition(currentSegmentInformation. to ) .lat();
    double to_Lon = getIntersectionPosition(currentSegmentInformation. to ) .lon();
    
    double from_X = x_from_lon(from_Lon);
    double from_Y = y_from_lat(from_Lat);
    double to_X =  x_from_lon(to_Lon);
    double to_Y =  y_from_lat(to_Lat);

    fromX.push_back(from_X);
    fromY.push_back(from_Y);
    toX.push_back(to_X);
    toY.push_back(to_Y);
    
    double delta_Y = to_Y - from_Y;
    double delta_X = to_X - from_X;

    if (delta_X != 0) {
    double angle = atan(delta_Y / delta_X) * (180/ M_PI) ;
    allAngle.push_back (angle);
    }
    else {
        allAngle.push_back(0);
    }
    string street_Name = getStreetName(getInfoStreetSegment(countStreetSegment).streetID);
    street_Names.push_back (street_Name);
}
   ezgl::rectangle initial_world({x_from_lon( min_lon), min_lat},{x_from_lon( max_lon), max_lat});
    
    cout <<"min x is: "<< x_from_lon( min_lon) <<endl;
    cout <<"min y is: "<< min_lat <<endl;
    cout <<"max x is: "<< x_from_lon( max_lon)<<endl;
    cout <<"max y is: " <<max_lat <<endl;
      globalPointer = &application;
  application.add_canvas("MainCanvas", draw_main_canvas, initial_world);//if_background_darker()
  
  application.run(initial_setup,act_on_mouse_click ,act_on_mouse_move ,act_on_key_press);
}





void draw_main_canvas(ezgl::renderer *g){
         if(open_night_mode == false){g->set_color(background);}
            else {g->set_color(ezgl::BLACK);}
    //g->set_color(ezgl::BLACK);
    g->fill_rectangle({-1000, -1000}, {1000, 1000});
    //loop through all the intersection points

    
    //Refresh current bounds of visible area
    ezgl::rectangle currentBounds;
    currentBounds = g->get_visible_world();
    
    //below is the colour the feature///////////////////////////////glen////
    int total_colour_needed_filled;
    total_colour_needed_filled = getNumFeatures();
    //this for loop is to loop through all feature in the database
    for(int feature_id = 0; feature_id < total_colour_needed_filled; feature_id++){
        int feature_total_points = getFeaturePointCount(feature_id);
        FeatureType feature_to_draw;
        feature_to_draw = getFeatureType(feature_id);
        
        vector<ezgl::point2d> feature_points;
        //this for loop is used to collect all the points of a feature
        LatLon first = getFeaturePoint(0, feature_id);
        LatLon second = getFeaturePoint((feature_total_points-1), feature_id);
        bool is_open;//to check if it is closed or open
//        if(first == second){
//            is_open = true;
//        }else if(first != second){ is_open == false;} 
        for(int current_point = 0; current_point < feature_total_points; current_point++){
            LatLon LatLon_point = getFeaturePoint(current_point, feature_id);
            float x = x_from_lon(LatLon_point.lon());
            float y = y_from_lat(LatLon_point.lat());
            feature_points.push_back({x,y});
        }
        if(feature_to_draw ==Island){
            if(open_night_mode == false){g->set_color(ezgl::GREY_55);}
            else {g->set_color(ezgl::GREY_75);}
            if (feature_points.size()>1)
            g->fill_poly(feature_points);
        }else if(feature_to_draw == Unknown){
            g->set_color(ezgl::LIGHT_PINK); 
            if (feature_points.size()>1){
//               if(is_open == true){        
//               }else 
               g->fill_poly(feature_points); 
            }
        }else if(feature_to_draw ==Lake){
            if(open_night_mode == false){g->set_color(ezgl::LIGHT_SKY_BLUE);}
            else {g->set_color(ezgl::DARK_SLATE_BLUE);}
            if (feature_points.size()>1)
            g->fill_poly(feature_points);
        }
        //Sets colour of feature depending on type
        else if(feature_to_draw ==Park){
            if(open_night_mode == false){g->set_color(ezgl::GREEN);}
            else {g->set_color(ezgl::DARK_GREEN);}
            if (feature_points.size()>1)
            g->fill_poly(feature_points);
        }else if(feature_to_draw ==Stream) {//stream is a case it self--open shape
            double streamWidth = streetWidthCheck(g, 1);
            g->set_line_width(streamWidth);
            if(open_night_mode == false) {g->set_color(ezgl::LIGHT_SKY_BLUE);}
            else {g->set_color(ezgl::DARK_SLATE_BLUE);} 
            if (feature_points.size()>1) {
                for (int i = 0; i < feature_points.size() - 1; i++){
                    g->draw_line({feature_points[i]} , {feature_points[i+1]});
                    }
                }
            
        }else if(feature_to_draw ==Beach){
            if(open_night_mode == false){g->set_color(ezgl::ORANGE);}
            else {g->set_color(ezgl::SADDLE_BROWN);}
            if (feature_points.size()>1)
            g->fill_poly(feature_points);
        }else if(feature_to_draw ==River){
            if(open_night_mode == false){g->set_color(ezgl::CYAN);}
            else {g->set_color(ezgl::DARK_SLATE_BLUE);}
            if (feature_points.size()>1)
                g->fill_poly(feature_points);
        }else if(feature_to_draw ==Building){
            if((currentBounds.right() - currentBounds.left()) <= 0.015) {
            if(open_night_mode == false){g->set_color(ezgl::GREY_75);}
            else {g->set_color(ezgl::ORANGE);}
            if (feature_points.size()>1)
            g->fill_poly(feature_points);
            }
        }else if(feature_to_draw ==Greenspace){
            if(open_night_mode == false){g->set_color(ezgl::LIME_GREEN);}
            else {g->set_color(ezgl::DARK_GREEN);}
            if (feature_points.size()>1)
            g->fill_poly(feature_points);
        }else if(feature_to_draw ==Golfcourse){
            if(open_night_mode == false){g->set_color(ezgl::GREEN);}
            else {g->set_color(ezgl::DARK_GREEN);}
            if (feature_points.size()>1)
            g->fill_poly(feature_points);
        }

        feature_points.clear();
     //above is the colour the feature//////////////////////////////////glen////    
        
    }
    
    
    
    
    
        //below is the colour the feature green area on island///////////////////////////////glen////
    //int total_colour_needed_filled;
    //total_colour_needed_filled = getNumFeatures();
    //this for loop is to loop through all feature in the database
    for(int feature_id = 0; feature_id < total_colour_needed_filled; feature_id++){
        int feature_total_points = getFeaturePointCount(feature_id);
        FeatureType feature_to_draw;
        feature_to_draw = getFeatureType(feature_id);
        
        vector<ezgl::point2d> feature_points;
        //this for loop is used to collect all the points of a feature
        LatLon first = getFeaturePoint(0, feature_id);
        LatLon second = getFeaturePoint((feature_total_points-1), feature_id);
        bool is_open;//to check if it is closed or open
//        if(first == second){
//            is_open = true;
//        }else if(first != second){ is_open == false;} 
        for(int current_point = 0; current_point < feature_total_points; current_point++){
            LatLon LatLon_point = getFeaturePoint(current_point, feature_id);
            float x = x_from_lon(LatLon_point.lon());
            float y = y_from_lat(LatLon_point.lat());
            feature_points.push_back({x,y});
        }

        //Sets colour of feature depending on type
        if(feature_to_draw ==Park){
            if(open_night_mode == false){g->set_color(ezgl::GREEN);}
            else {g->set_color(ezgl::DARK_GREEN);}
            if (feature_points.size()>1)
            g->fill_poly(feature_points);
        }else if(feature_to_draw ==Stream) {//stream is a case it self--open shape
            double streamWidth = streetWidthCheck(g, 1);
            g->set_line_width(streamWidth);
            if(open_night_mode == false) {g->set_color(ezgl::LIGHT_SKY_BLUE);}
            else {g->set_color(ezgl::DARK_SLATE_BLUE);} 
            if (feature_points.size()>1) {
                for (int i = 0; i < feature_points.size() - 1; i++){
                    g->draw_line({feature_points[i]} , {feature_points[i+1]});
                    }
                }
            
        }else if(feature_to_draw ==Greenspace){
            if(open_night_mode == false){g->set_color(ezgl::LIME_GREEN);}
            else {g->set_color(ezgl::DARK_GREEN);}
            if (feature_points.size()>1)
            g->fill_poly(feature_points);
        }else if(feature_to_draw ==Golfcourse){
            if(open_night_mode == false){g->set_color(ezgl::GREEN);}
            else {g->set_color(ezgl::DARK_GREEN);}
            if (feature_points.size()>1)
            g->fill_poly(feature_points);
        }

        feature_points.clear();
     //above is the colour the feature green area//////////////////////////////////glen////    

    }
    
    
    
    
    
    
    
//    //draw all the segments////////////////////////////////////////////////////
    int total_segments = getNumStreetSegments();
    for(int id=0; id < total_segments; id++ ){

        //if the segment has zero curve points
        double x1;
        double y1;
        double x2;
        double y2;
        double xtotal = 0;
        double ytotal = 0;
        double streetWidth = streetWidthCheck(g, 1);
        int point_Count = getInfoStreetSegment(id).curvePointCount; 
        
        string current_street_name = getStreetName(getInfoStreetSegment(id).streetID);
        
 //if there is no curve point, call the function of distance between two points directly        
        if(getInfoStreetSegment(id).curvePointCount == 0) { 
            InfoStreetSegment currentSegment = getInfoStreetSegment(id);
            LatLon point1 = getIntersectionPosition(currentSegment.from);
            LatLon point2 = getIntersectionPosition(currentSegment.to);
            x1 = x_from_lon(point1.lon());
            y1 = y_from_lat(point1.lat());
            x2 = x_from_lon(point2.lon());
            y2 = y_from_lat(point2.lat());
            
            
            if(current_street_name == existing_street_name){//check the find correct input first
                g->set_color(ezgl::RED);
                g->set_line_width(streetWidth*6);
            }else{   //if no correct input 
                if (highwayCheck(currentSegment) == true) {
                    g->set_color(ezgl::YELLOW);
                    g->set_line_width(streetWidth*2);
                    }
                if (biggestRoadCheck(currentSegment) == true) {
                    g->set_color(ezgl::PINK);
                    g->set_line_width(streetWidth*2);
                    }
                else {
                    if (open_night_mode == true)
                        {
                        g->set_color(128, 128, 128, 255);
                    }
                    else {
                        g->set_color(ezgl::WHITE);
                    }
                    g->set_line_width(streetWidth);
                    }      
            }
                g->draw_line({x1, y1}, {x2, y2});

                
                //Draws street name, size is based on zoom level and street distance (so the name doesn't overextend outside the street)
                double xavg = (x1 + x2)/2;
                double yavg = (y1 + y2)/2;
                if (current_street_name != "<unknown>") {
                    if (currentBounds.right() - currentBounds.left() < 0.002 ||
                       ((currentBounds.right() - currentBounds.left() < 0.0045) && 
                            find_distance_between_two_points(make_pair(point1, point2)) > 70) ||
                       ((currentBounds.right() - currentBounds.left() < 0.01) && 
                            find_distance_between_two_points(make_pair(point1, point2)) > 130))     
                    {
                    if (currentBounds.right() - currentBounds.left() < 0.002) {
                        g->set_font_size(15);    
                    }
                    else if (currentBounds.right() - currentBounds.left() < 0.0045) {
                        g->set_font_size(10);
                    }
                    else {
                        g->set_font_size(7);
                    }
                    g->set_text_rotation(allAngle[id]);
                    if (getInfoStreetSegment(id).oneWay == true) {
                        g->set_color(ezgl::PURPLE);
                    }
                    else {
                        if(open_night_mode == true){
                            g->set_color(ezgl::WHITE);
                        }
                        else {
                        g->set_color(ezgl::BLACK);
                        }
                    }
                    g->draw_text({xavg, yavg}, current_street_name);
                    }
                }
                
        } else { //if there are curve points//my method: make it three segments
            LatLon point1 = getIntersectionPosition(getInfoStreetSegment(id).from);
            x1 = x_from_lon(point1.lon());
            y1 = y_from_lat(point1.lat());
            LatLon point2 = getStreetSegmentCurvePoint(0, id);
            x2 = x_from_lon(point2.lon());
            y2 = y_from_lat(point2.lat());
            xtotal = xtotal + x1 + x2;
            ytotal = ytotal + y1 + y2;
            if(current_street_name == existing_street_name){//check the find correct input first
                g->set_color(ezgl::RED);
                g->set_line_width(streetWidth*6);
            }else{          
            
            
                if (highwayCheck(getInfoStreetSegment(id)) == true) {
                    if (open_night_mode == true)
                        {
                        g->set_color(180, 180, 0, 255);
                        }
                    else
                        {
                        g->set_color(ezgl::YELLOW);
                        }
                    }
                else {
                    if (open_night_mode == true)
                        {
                        g->set_color(128, 128, 128, 255);
                    }
                    else {
                        g->set_color(ezgl::WHITE);
                    }
                }
            }
            g->draw_line({x1, y1}, {x2, y2});

            for (int k=0; k< (getInfoStreetSegment(id).curvePointCount)-1; k++){
                point1 = getStreetSegmentCurvePoint(k, id); 
                point2 = getStreetSegmentCurvePoint(k+1, id);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                xtotal = xtotal + x2;
                ytotal = ytotal + y2;
                g->draw_line({x1, y1}, {x2, y2});
            }
            point1 = getStreetSegmentCurvePoint(point_Count-1, id); 
            point2 = getIntersectionPosition(getInfoStreetSegment(id).to);
            x1 = x_from_lon(point1.lon());
            y1 = y_from_lat(point1.lat());             
            x2 = x_from_lon(point2.lon());
            y2 = y_from_lat(point2.lat());
            xtotal = xtotal + x2;
            ytotal = ytotal + y2;
            g->draw_line({x1, y1}, {x2, y2}); 
            
            //Draw name of the curve, if the name does not return "<unknown>"
            if (current_street_name != "<unknown>") {
                if (currentBounds.right() - currentBounds.left() < 0.002 ||
                   ((currentBounds.right() - currentBounds.left() < 0.0045) && 
                        find_distance_between_two_points(make_pair(point1, point2)) >70) ||
                   ((currentBounds.right() - currentBounds.left() < 0.01) && 
                        find_distance_between_two_points(make_pair(point1, point2)) > 130))     
                    {
                    
                    if (currentBounds.right() - currentBounds.left() < 0.002) {
                        g->set_font_size(15);    
                    }
                    else if (currentBounds.right() - currentBounds.left() < 0.0045) {
                        g->set_font_size(10);
                    }
                    else {
                        g->set_font_size(7);
                    }
                    g->set_text_rotation(allAngle[id]);
                    if (getInfoStreetSegment(id).oneWay == true) {
                        g->set_color(ezgl::PURPLE);
                    }
                    else {
                        if(open_night_mode == true){
                            g->set_color(ezgl::WHITE);
                        }
                        else {
                        g->set_color(ezgl::BLACK);
                        }
                    }
                    double averageX = xtotal / (getInfoStreetSegment(id).curvePointCount + 2);
                    double averageY = ytotal / (getInfoStreetSegment(id).curvePointCount + 2);
                    g->draw_text({averageX, averageY}, current_street_name);
                    }
                } 
        }
    }  
    
    //Write down names of each POI
    if ((currentBounds.right() - currentBounds.left()) < 0.008) {
         drawPOI_Name(g);  
    }
    
    for (size_t i = 0; i < intersections.size(); ++i) {
        float x = x_from_lon(intersections[i].position.lon());
        float y = y_from_lat(intersections[i].position.lat());
        float width = 0.00005;
        float height = width;
        float start_angle = 0;
        float end_angle = 360;
        
        

        
        if (intersections[i].highlight) {
            g->set_color(ezgl::RED);
        } else {
            g->set_color(180, 180, 180, 255);
        }
        
        
        
        //draw the intersection with a filled circle
        g-> ezgl::renderer::fill_elliptic_arc(ezgl::point2d(x, y), width, width, start_angle, end_angle );
        
                if(i == flag_for_found_intersection_by_find_bottom ){
        //draw png
          ezgl::surface *png_surface = ezgl::renderer::load_png("small_image.png");
  g->draw_surface(png_surface, ezgl::point2d(x, y));
  ezgl::renderer::free_surface(png_surface);
        }
        
        
    }
    
    
    
    
    //////////////////////////////////////////////////////////////////////////
    //////                                                              //////
    //////     draw path by two clicks                                  //////
    //////                                                              //////
    //////////////////////////////////////////////////////////////////////////

    
    //if(click_2_intersections_bottom_pressed = true){
        int first, second;
        first = mouse_clicked_1st;
        second = mouse_clicked_2nd; 
   // }
 //if(click_2_intersections_bottom_pressed = true){

//   int first, second;
//    if(click_2_intersections_bottom_pressed = true){
//            first = mouse_clicked_1st;
//            second = mouse_clicked_2nd; 
//    }


                
     //  /*  
        vector<int > test = find_path_between_intersections( first,  second,  15);
        g->set_color(ezgl::ORANGE);
        cout<<endl << endl <<endl<<endl << endl <<endl<<endl<<endl << endl <<endl;
        cout<< "you will pass the following streets, and turn at the intersection"
                " between every two adjacent street names" <<endl;
        vector<int> path ;
        //for(int i = 0, i < test.size(); i++ ){
        string existing ="nothing";
        for(int id=0; id < test.size(); id++ ){

            //if the segment has zero curve points
            double x1;
            double y1;
            double x2;
            double y2;
            double xtotal = 0;
            double ytotal = 0;

            int point_Count = getInfoStreetSegment(test[id]).curvePointCount; 

            if(getStreetName(getInfoStreetSegment(test[id]).streetID) == existing ){
                ;
            }else{
                cout << getStreetName(getInfoStreetSegment(test[id]).streetID) << endl;
            }


            existing = getStreetName(getInfoStreetSegment(test[id]).streetID);


     //if there is no curve point, call the function of distance between two points directly        
            if(getInfoStreetSegment(test[id]).curvePointCount == 0) { 
                InfoStreetSegment currentSegment = getInfoStreetSegment(test[id]);
                LatLon point1 = getIntersectionPosition(currentSegment.from);
                LatLon point2 = getIntersectionPosition(currentSegment.to);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                    g->draw_line({x1, y1}, {x2, y2});

            } else { //if there are curve points//my method: make it three segments
                LatLon point1 = getIntersectionPosition(getInfoStreetSegment(test[id]).from);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());
                LatLon point2 = getStreetSegmentCurvePoint(0, test[id]);
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                xtotal = xtotal + x1 + x2;
                ytotal = ytotal + y1 + y2;

                g->draw_line({x1, y1}, {x2, y2});

                for (int k=0; k< (getInfoStreetSegment(test[id]).curvePointCount)-1; k++){
                    point1 = getStreetSegmentCurvePoint(k, test[id]); 
                    point2 = getStreetSegmentCurvePoint(k+1, test[id]);
                    x1 = x_from_lon(point1.lon());
                    y1 = y_from_lat(point1.lat());
                    x2 = x_from_lon(point2.lon());
                    y2 = y_from_lat(point2.lat());
                    xtotal = xtotal + x2;
                    ytotal = ytotal + y2;
                    g->draw_line({x1, y1}, {x2, y2});
                }
                point1 = getStreetSegmentCurvePoint(point_Count-1, test[id]); 
                point2 = getIntersectionPosition(getInfoStreetSegment(test[id]).to);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());             
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                xtotal = xtotal + x2;
                ytotal = ytotal + y2;
                g->draw_line({x1, y1}, {x2, y2}); 
            }
        }
//        click_2_intersections_bottom_pressed = false;
//    }else{
//        ;// do nothing// after testing, even if checking the bool variable makes no difference 
//    }

    
    
   // */

 //} 
    //////////////////////////////////////////////////////////////////////////
    //////                                                              //////
    //////     draw path by 4 input street names                        //////
    //////                                                              //////
    //////////////////////////////////////////////////////////////////////////

    
    //if(click_2_intersections_bottom_pressed = true){
        int first_search, second_search;
        first_search = id_1st_intersection_by_search;
        second_search = id_2nd_intersection_by_search; 
   // }
 //if(click_2_intersections_bottom_pressed = true){

//   int first, second;
//    if(click_2_intersections_bottom_pressed = true){
//            first = mouse_clicked_1st;
//            second = mouse_clicked_2nd; 
//    }


                
     //  /*  
        vector<int> test_search = find_path_between_intersections( first_search, 
        second_search,  0);
        g->set_color(ezgl::PINK);
        cout<<endl << endl <<endl<<endl << endl <<endl<<endl<<endl << endl <<endl;
        cout<< "you will pass the following streets, and turn at the intersection "
                "between every two adjacent street names" <<endl;
        //vector<int> path ;
        //for(int i = 0, i < test.size(); i++ ){
         string existing_search ="nothing";
        for(int id=0; id < test_search.size(); id++ ){

            //if the segment has zero curve points
            double x1;
            double y1;
            double x2;
            double y2;
            double xtotal = 0;
            double ytotal = 0;

            int point_Count = getInfoStreetSegment(test_search[id]).curvePointCount; 

            if(getStreetName(getInfoStreetSegment(test_search[id]).streetID) == existing_search ){
                ;
            }else{
                cout << getStreetName(getInfoStreetSegment(test_search[id]).streetID) << endl;
            }


            existing_search = getStreetName(getInfoStreetSegment(test_search[id]).streetID);


     //if there is no curve point, call the function of distance between two points directly        
            if(getInfoStreetSegment(test_search[id]).curvePointCount == 0) { 
                InfoStreetSegment currentSegment = getInfoStreetSegment(test_search[id]);
                LatLon point1 = getIntersectionPosition(currentSegment.from);
                LatLon point2 = getIntersectionPosition(currentSegment.to);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                    g->draw_line({x1, y1}, {x2, y2});

            } else { //if there are curve points//my method: make it three segments
                LatLon point1 = getIntersectionPosition(getInfoStreetSegment(test_search[id]).from);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());
                LatLon point2 = getStreetSegmentCurvePoint(0, test_search[id]);
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                xtotal = xtotal + x1 + x2;
                ytotal = ytotal + y1 + y2;

                g->draw_line({x1, y1}, {x2, y2});

                for (int k=0; k< (getInfoStreetSegment(test_search[id]).curvePointCount)-1; k++){
                    point1 = getStreetSegmentCurvePoint(k, test_search[id]); 
                    point2 = getStreetSegmentCurvePoint(k+1, test_search[id]);
                    x1 = x_from_lon(point1.lon());
                    y1 = y_from_lat(point1.lat());
                    x2 = x_from_lon(point2.lon());
                    y2 = y_from_lat(point2.lat());
                    xtotal = xtotal + x2;
                    ytotal = ytotal + y2;
                    g->draw_line({x1, y1}, {x2, y2});
                }
                point1 = getStreetSegmentCurvePoint(point_Count-1, test_search[id]); 
                point2 = getIntersectionPosition(getInfoStreetSegment(test_search[id]).to);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());             
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                xtotal = xtotal + x2;
                ytotal = ytotal + y2;
                g->draw_line({x1, y1}, {x2, y2}); 
            }
        }
         
  
         
    //////////////////////////////////////////////////////////////////////////
    //////                                                              //////
    //////     draw walk and drive path by 4 input street names         //////
    //////                                                              //////
    //////////////////////////////////////////////////////////////////////////

 
    //if(click_2_intersections_bottom_pressed = true){
        //int first_search, second_search;
        first_search = id_1st_intersection_by_search;
        second_search = id_2nd_intersection_by_search; 
   // }
 //if(click_2_intersections_bottom_pressed = true){

//   int first, second;
//    if(click_2_intersections_bottom_pressed = true){
//            first = mouse_clicked_1st;
//            second = mouse_clicked_2nd; 
//    }


                
     //  /*  
        pair<std::vector<int>, std::vector<int>>  walk_drive_test_search = 
        find_path_with_walk_to_pick_up( first_search,  second_search, 15, 
        global_walking_speed , global_walking_time_limit );
        g->set_color(ezgl::BLUE);
        cout<<endl << endl <<endl<<endl << endl <<endl<<endl<<endl << endl <<endl;
        cout<< "you will pass the following streets, and turn at the intersection "
                "between every two adjacent street names" <<endl;
        //vector<int> path ;
        //for(int i = 0, i < test.size(); i++ ){
         existing_search ="nothing";
         vector<int> collection_id_for_walk = walk_drive_test_search.first;
         vector<int> collection_id_for_drive = walk_drive_test_search.second;
        //the first for loop print walk path in blue
        for(int id=0; id < collection_id_for_walk.size(); id++ ){

            //if the segment has zero curve points
            double x1;
            double y1;
            double x2;
            double y2;
            double xtotal = 0;
            double ytotal = 0;

            int point_Count = getInfoStreetSegment(collection_id_for_walk[id]).curvePointCount; 

            if(getStreetName(getInfoStreetSegment(collection_id_for_walk[id]).streetID)
                    == existing_search ){
                ;
            }else{
                cout << getStreetName(getInfoStreetSegment(collection_id_for_walk[id]).streetID)
                        << endl;
            }


            existing_search = getStreetName(getInfoStreetSegment(collection_id_for_walk[id]).streetID);


     //if there is no curve point, call the function of distance between two points directly        
            if(getInfoStreetSegment(collection_id_for_walk[id]).curvePointCount == 0) { 
                InfoStreetSegment currentSegment = getInfoStreetSegment(collection_id_for_walk[id]);
                LatLon point1 = getIntersectionPosition(currentSegment.from);
                LatLon point2 = getIntersectionPosition(currentSegment.to);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                    g->draw_line({x1, y1}, {x2, y2});

            } else { //if there are curve points//my method: make it three segments
                LatLon point1 = getIntersectionPosition(getInfoStreetSegment(
                        collection_id_for_walk[id]).from);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());
                LatLon point2 = getStreetSegmentCurvePoint(0, collection_id_for_walk[id]);
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                xtotal = xtotal + x1 + x2;
                ytotal = ytotal + y1 + y2;

                g->draw_line({x1, y1}, {x2, y2});

                for (int k=0; k< (getInfoStreetSegment(collection_id_for_walk[id])
                        .curvePointCount)-1; k++){
                    point1 = getStreetSegmentCurvePoint(k, collection_id_for_walk[id]); 
                    point2 = getStreetSegmentCurvePoint(k+1, collection_id_for_walk[id]);
                    x1 = x_from_lon(point1.lon());
                    y1 = y_from_lat(point1.lat());
                    x2 = x_from_lon(point2.lon());
                    y2 = y_from_lat(point2.lat());
                    xtotal = xtotal + x2;
                    ytotal = ytotal + y2;
                    g->draw_line({x1, y1}, {x2, y2});
                }
                point1 = getStreetSegmentCurvePoint(point_Count-1, collection_id_for_walk[id]); 
                point2 = getIntersectionPosition(getInfoStreetSegment(
                        collection_id_for_walk[id]).to);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());             
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                xtotal = xtotal + x2;
                ytotal = ytotal + y2;
                g->draw_line({x1, y1}, {x2, y2}); 
            }
        }        
        ////////////////////////////////////////////////////////////////////// 
        //the second for loop print the drive path in thistle
         g->set_color(ezgl::THISTLE);
                 for(int id=0; id < collection_id_for_drive.size(); id++ ){

            //if the segment has zero curve points
            double x1;
            double y1;
            double x2;
            double y2;
            double xtotal = 0;
            double ytotal = 0;

            int point_Count = getInfoStreetSegment(collection_id_for_drive[id]).curvePointCount; 

            if(getStreetName(getInfoStreetSegment(collection_id_for_drive[id]).streetID)
                    == existing_search ){
                ;
            }else{
                cout << getStreetName(getInfoStreetSegment(collection_id_for_drive[id]).
                        streetID) << endl;
            }


            existing_search = getStreetName(getInfoStreetSegment(
                    collection_id_for_drive[id]).streetID);


     //if there is no curve point, call the function of distance between two points directly        
            if(getInfoStreetSegment(collection_id_for_drive[id]).curvePointCount == 0) { 
                InfoStreetSegment currentSegment = getInfoStreetSegment(collection_id_for_drive[id]);
                LatLon point1 = getIntersectionPosition(currentSegment.from);
                LatLon point2 = getIntersectionPosition(currentSegment.to);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                    g->draw_line({x1, y1}, {x2, y2});

            } else { //if there are curve points//my method: make it three segments
                LatLon point1 = getIntersectionPosition(getInfoStreetSegment(
                        collection_id_for_drive[id]).from);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());
                LatLon point2 = getStreetSegmentCurvePoint(0, collection_id_for_drive[id]);
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                xtotal = xtotal + x1 + x2;
                ytotal = ytotal + y1 + y2;

                g->draw_line({x1, y1}, {x2, y2});

                for (int k=0; k< (getInfoStreetSegment(collection_id_for_drive[id]).curvePointCount)-1; k++){
                    point1 = getStreetSegmentCurvePoint(k, collection_id_for_drive[id]); 
                    point2 = getStreetSegmentCurvePoint(k+1, collection_id_for_drive[id]);
                    x1 = x_from_lon(point1.lon());
                    y1 = y_from_lat(point1.lat());
                    x2 = x_from_lon(point2.lon());
                    y2 = y_from_lat(point2.lat());
                    xtotal = xtotal + x2;
                    ytotal = ytotal + y2;
                    g->draw_line({x1, y1}, {x2, y2});
                }
                point1 = getStreetSegmentCurvePoint(point_Count-1, collection_id_for_drive[id]); 
                point2 = getIntersectionPosition(getInfoStreetSegment(collection_id_for_drive[id]).to);
                x1 = x_from_lon(point1.lon());
                y1 = y_from_lat(point1.lat());             
                x2 = x_from_lon(point2.lon());
                y2 = y_from_lat(point2.lat());
                xtotal = xtotal + x2;
                ytotal = ytotal + y2;
                g->draw_line({x1, y1}, {x2, y2}); 
            }
        }        
         
         
         
}

//Sometimes, duplicate streets in the street name vectors causes a loop as it
//cannot choose between the two duplicates. Try and fix duplicates.
void find_button(GtkWidget */*widget*/, ezgl::application *application) {

//    // Update the status bar message
    application->update_message("Find button pressed");

   
    string firstStreet;
    string secondStreet;
    
    bool firstIsFound = false;
    bool secondIsFound = false;
    
    cout << ">>Please enter name of first street:";
    getline(cin, firstStreet);
    cin.ignore(1000, '\n');
    
    cout << ">>Please enter name of second street:";
    getline(cin, secondStreet);
    cin.ignore(1000, '\n');
    
    vector <int> firstMatch = find_street_ids_from_partial_street_name(firstStreet);
    vector <int> secondMatch = find_street_ids_from_partial_street_name(secondStreet);

    while (firstMatch.size() != 1 && secondMatch.size() != 1) {
        if (firstMatch.size() != 1) {
            if (firstMatch.size() == 0) {
                cout << ">>No match was found. Please try again. " << endl;

            }
            if (firstMatch.size() > 1) {
                cout << ">>Below are results for the first street: " << endl;
                for (int i = 0; i < firstMatch.size(); i++) {
                    cout << getStreetName(firstMatch[i]) << endl;


                }

                cout << ">>Please specify desired street. " << endl;


            }
        } else {
            firstIsFound = true;
        }

        if (secondMatch.size() != 1) {



            if (secondMatch.size() == 0) {
                cout << ">>No match was found. Please try again. " << endl;

            }

            if (secondMatch.size() > 1) {
                cout << ">>Below are results for the second street:" << endl;
                for (int i = 0; i < secondMatch.size(); i++) {
                    cout << getStreetName(secondMatch[i]) << endl;

                }

                cout << ">>Please specify desired street. " << endl;
            }
        } else {
            secondIsFound = true;
        }
        
        if (firstIsFound == false) {
            cout << ">>Please enter name of first street: ";
            getline(cin, firstStreet);
            cin.ignore(1000, '\n');
            firstMatch.resize(find_street_ids_from_partial_street_name(firstStreet).size());
            firstMatch = find_street_ids_from_partial_street_name(firstStreet);
        } else {
            cout << ">>Found match for first street: " << firstMatch[0] << endl;

        }
        if (secondIsFound == false) {

            cout << ">>Please enter name of second street: ";
            getline(cin, secondStreet);
            cin.ignore(1000, '\n');
            secondMatch.resize(find_street_ids_from_partial_street_name(secondStreet).size());

            secondMatch = find_street_ids_from_partial_street_name(secondStreet);
        } else {
            cout << ">Found match for second street: " << secondMatch[0] << endl;
        }

    }
    
    
    pair<int, int> streets;
    streets.first = firstMatch[0];
    streets.second = secondMatch[0];
    
    vector <int> intersectionsBetween = find_intersections_of_two_streets(streets);
    
    if (intersectionsBetween.size() == 0) {
        cout << ">There are no intersections between " << getStreetName(firstMatch[0])
                << " and " << getStreetName(secondMatch[0]) << endl;
    } else if (intersectionsBetween.size() == 1) {
        // this is flag for draw
        flag_for_found_intersection_by_find_bottom = intersectionsBetween[0];
        
        cout << ">The intersection between " << getStreetName(firstMatch[0]) << 
                " and " << getStreetName(secondMatch[0]) << "is: " << intersectionsBetween[0] << endl;
        intersections[intersectionsBetween[0]].highlight = true;

    } else {
        cout << ">The intersections between " << getStreetName(firstMatch[0]) 
                << " and " << getStreetName(secondMatch[0]) << " are: ";
        for (int j = 0; j < intersectionsBetween.size(); j++) {
            // this is flag for draw
            flag_for_found_intersection_by_find_bottom = intersectionsBetween[j];
            
            cout << getIntersectionName(intersectionsBetween[j]) << endl;
            intersections[intersectionsBetween[j]].highlight = true;
        }
   
    
}   
//    if (intersectionsBetween.size() != 0) {
//        ezgl::renderer* g = application->get_renderer();
//        ezgl::rectangle rec = findMaxRectanglesOfIntersections(intersectionsBetween);
//        g->set_visible_world(rec);
//        application->refresh_drawing();
//    }
 }

void act_on_mouse_click(ezgl::application* app,   GdkEventButton* event,
                        double x, double y) {
  std::cout << "Mouse clicked at (" << x << "," << y << ")\n";

  LatLon pos = LatLon(lat_from_y(y), lon_from_x(x)); 
  int id = find_closest_intersection(pos); 
  ////////////adding mouse click sequence///////////////////////////////////////
  if(click_odd_times == true){
      mouse_clicked_1st = id;
      click_odd_times = false;
  }else{
      mouse_clicked_2nd = id;
      click_odd_times = true;
  }
    //////////adding mouse click sequence/////////////////////////////////////// 
    
  cout << "ID:" << id << endl;
  std::cout << "Closest Intersection:" << intersections[id].name << "\n"; 
  cout << "Distance in metres" << find_distance_between_two_points(make_pair(pos, 
    getIntersectionPosition(id)));
  //Reset highlight for each intersection, enable highlight for closest intersection
  for (int i = 0; i < intersections.size(); i++) {
      intersections[i].highlight = false;
  }
  intersections[id].highlight = true;
  
  app->refresh_drawing();
} 

float x_from_lon( float lon){
     float x_value = lon * cos(latAvg * DEGREE_TO_RADIAN);
     return x_value;
}

float y_from_lat( float lat){
     float y_value = lat;
     return y_value;
}

float lon_from_x( float x){
    float lon = x/cos(latAvg * DEGREE_TO_RADIAN);
    return lon;
}

float lat_from_y( float y){
    return y;
}




void initial_setup(ezgl::application *application, bool /*new_window*/)
{

  // Create a Test button and link it with test_button callback fn.
    application->create_button("Find ", 8, find_button);
  application->create_button("Highlight Street", 8, test_FIND_button);
   application->create_button("Night Mode", 8, test_night_mode_button);
   application->create_button("Day Mode", 8, test_day_mode_button);

   application->create_button("How to use the UI", 8, learn_UI_button);
   application->create_button("Find path by clicking at 2 locations", 8, 
   click_2_intersections_bottom);
   application->create_button("Find path by using the search bar", 8, 
   find_path_by_typing_into_search_bar_button);
   application->create_button("Find walk & drive path by search bar", 
   8, find_walk_drive_path_by_typing_into_search_bar_button);
}



void find_walk_drive_path_by_typing_into_search_bar_button(GtkWidget */*Widget*/,
        ezgl::application *application)
{
    //not sure if i hsould make it global variable yet
   /// string command;
    
    
  // application -> update_message("Find button pressed");
   //application -> refresh_drawing();
         GtkEntry* text_entry = (GtkEntry *) application->get_object("TextInput");
    const char* text = gtk_entry_get_text(text_entry);
    walk_drive_four_street_name = text;
    string how_to_use = "type into search bar in the form: streetA & streetB -> streetC & streetD ";
    application->update_message(how_to_use);
    
    //stringstream lineStream (text);
    //lineStream  >> command;
    string streetA, streetB, streetC, streetD, temp_string_1, temp_string_2, temp_string_3;
    streetA = walk_drive_four_street_name.substr(0, walk_drive_four_street_name.find(" & "));
    int pos = walk_drive_four_street_name.find(" & ")+3;
    temp_string_1 = walk_drive_four_street_name.substr(pos);
    streetB = temp_string_1.substr(0, temp_string_1.find(" -> ") );
    pos = temp_string_1.find(" -> ")+3;
    temp_string_2 = temp_string_1.substr(pos);
    streetC = temp_string_2.substr(0, temp_string_2.find(" & ") );
    
    pos = temp_string_2.find(" & ")+3;
    temp_string_3 = temp_string_2.substr(pos);
    streetD = temp_string_3.substr(0, temp_string_3.find(" & ") );
    cout<<"//////testing teh find path by searching bar/////////////////" <<endl;
    cout <<"streetA   =" << streetA <<endl;
   // cout <<"temp_string_1   =" << temp_string_1 <<endl;
    cout <<"streetB   =" << streetB <<endl;
   // cout <<"temp_string_2   =" << temp_string_2 <<endl;
    cout <<"streetC   =" << streetC <<endl;
    cout <<"streetD   =" << streetD <<endl;
    cout<<"//////testing teh find path by searching bar/////////////////" <<endl;
    
    
    // take in the walking speed and walking time limit
    int walking_speed;
    int walking_time_limit;
    
    //bool firstIsFound = false;
    //bool secondIsFound = false;
    
    cout << ">>Please enter number of walking_speed:";
    cin >> walking_speed;
    //getline(cin, walking_speed);
    //cin.ignore(1000, '\n');
    
    cout << ">>Please enter number of walking_time_limit:";
    cin >> walking_time_limit;
    //getline(cin, secondStreet);
    //cin.ignore(1000, '\n');
    global_walking_speed = walking_speed;
    global_walking_time_limit = walking_time_limit;
    
    
    //right now, i need to pass all the name to find_partial_name for error message
    vector<int> search_for_A = find_street_ids_from_partial_street_name(streetA);
    if(search_for_A.size() == 0 ){cout<<"the input for street name A is invalid, "
            "please try again"<<endl; }
    
    vector<int> search_for_B = find_street_ids_from_partial_street_name(streetB);
    if(search_for_B.size() == 0 ){cout<<"the input for street name B is invalid, "
            "please try again"<<endl; }
    
    vector<int> search_for_C = find_street_ids_from_partial_street_name(streetC);
    if(search_for_C.size() == 0 ){cout<<"the input for street name C is invalid,"
            "please try again"<<endl; }
    
    vector<int> search_for_D = find_street_ids_from_partial_street_name(streetD);
    if(search_for_D.size() == 0 ){cout<<"the input for street name D is invalid,"
            " please try again"<<endl; }
    
    //if all 4 names are unique
    if((search_for_A.size() == 1 )&&(search_for_B.size() == 1 ) && 
            (search_for_C.size() == 1 )&& (search_for_D.size() == 1 )){
        int id_A = search_for_A[0];
        int id_B = search_for_B[0];
        int id_C = search_for_C[0];
        int id_D = search_for_D[0];
        
 
        if((find_intersections_of_two_streets(make_pair(id_A,id_B)).size() > 0 )
                &&((find_intersections_of_two_streets(make_pair(id_C,id_D)).size() > 0 ))){
        //I assumed only one intersection between two streets
        id_1st_intersection_by_search = find_intersections_of_two_streets(make_pair(id_A,id_B))[0];
        id_2nd_intersection_by_search = find_intersections_of_two_streets(make_pair(id_C,id_D))[0];
        
        
        }
    }
    
    
    
    application->refresh_drawing();
} 


void find_path_by_typing_into_search_bar_button(GtkWidget */*Widget*/, 
        ezgl::application *application)
{
    //not sure if i hsould make it global variable yet
   /// string command;
    
    
  // application -> update_message("Find button pressed");
   //application -> refresh_drawing();
         GtkEntry* text_entry = (GtkEntry *) application->get_object("TextInput");
    const char* text = gtk_entry_get_text(text_entry);
    four_street_name = text;
    string how_to_use = "type into search bar in the form: streetA & streetB -> streetC & streetD";
    application->update_message(how_to_use);
    
    //stringstream lineStream (text);
    //lineStream  >> command;
    string streetA, streetB, streetC, streetD, temp_string_1, temp_string_2, temp_string_3;
    streetA = four_street_name.substr(0, four_street_name.find(" & "));
    int pos = four_street_name.find(" & ")+3;
    temp_string_1 = four_street_name.substr(pos);
    streetB = temp_string_1.substr(0, temp_string_1.find(" -> ") );
    pos = temp_string_1.find(" -> ")+3;
    temp_string_2 = temp_string_1.substr(pos);
    streetC = temp_string_2.substr(0, temp_string_2.find(" & ") );
    
    pos = temp_string_2.find(" & ")+3;
    temp_string_3 = temp_string_2.substr(pos);
    streetD = temp_string_3.substr(0, temp_string_3.find(" & ") );
    cout<<"//////testing teh find path by searching bar/////////////////" <<endl;
    cout <<"streetA   =" << streetA <<endl;
   // cout <<"temp_string_1   =" << temp_string_1 <<endl;
    cout <<"streetB   =" << streetB <<endl;
   // cout <<"temp_string_2   =" << temp_string_2 <<endl;
    cout <<"streetC   =" << streetC <<endl;
    cout <<"streetD   =" << streetD <<endl;
    cout<<"//////testing teh find path by searching bar//////////////////" <<endl;
    
    //right now, i need to pass all the name to find_partial_name for error message
    vector<int> search_for_A = find_street_ids_from_partial_street_name(streetA);
    if(search_for_A.size() == 0 ){cout<<"the input for street name A is invalid,"
            " please try again"<<endl; }
    
    vector<int> search_for_B = find_street_ids_from_partial_street_name(streetB);
    if(search_for_B.size() == 0 ){cout<<"the input for street name B is invalid,"
            " please try again"<<endl; }
    
    vector<int> search_for_C = find_street_ids_from_partial_street_name(streetC);
    if(search_for_C.size() == 0 ){cout<<"the input for street name C is invalid, "
            "please try again"<<endl; }
    
    vector<int> search_for_D = find_street_ids_from_partial_street_name(streetD);
    if(search_for_D.size() == 0 ){cout<<"the input for street name D is invalid,"
            " please try again"<<endl; }
    
    //if all 4 names are unique
    if((search_for_A.size() == 1 )&&(search_for_B.size() == 1 ) && 
            (search_for_C.size() == 1 )&& (search_for_D.size() == 1 )){
        int id_A = search_for_A[0];
        int id_B = search_for_B[0];
        int id_C = search_for_C[0];
        int id_D = search_for_D[0];
        
        cout << "id_A" << id_A <<endl;
        cout << "id_B"<< id_B <<endl;
        cout << "id_C"<< id_C <<endl;
        cout << "id_D"<< id_D <<endl;
        
        if((find_intersections_of_two_streets(make_pair(id_A,id_B)).size() > 0 )
        &&((find_intersections_of_two_streets(make_pair(id_C,id_D)).size() > 0 ))){
            //I assumed only one intersection between two streets
            id_1st_intersection_by_search = find_intersections_of_two_streets(make_pair(id_A,id_B))[0];
            cout << "id_1st_intersection_by_search"<< id_1st_intersection_by_search <<endl;
            id_2nd_intersection_by_search = find_intersections_of_two_streets(make_pair(id_C,id_D))[0];
            //id_2nd_intersection_by_search = find_intersections_of_two_streets(make_pair(id_A,id_B))[0];
            cout << "id_2nd_intersection_by_search"<< id_2nd_intersection_by_search <<endl;
        }

    }
    
    
    
    application->refresh_drawing();
}


void click_2_intersections_bottom(GtkWidget */*Widget*/, ezgl::application *application)
{
    application -> update_message("please click at two intersections at our map" );
    cout << " please click at two intersections at our map" <<endl;

    //click_2_intersections_bottom_pressed = false;

  // application -> update_message("Find button pressed");
   //application -> refresh_drawing();
         //GtkEntry* text_entry = (GtkEntry *) application->get_object("TextInput");
    //const char* text = gtk_entry_get_text(text_entry);
    

    //application->update_message(text);
    //existing_street_name = text;
    
    application->refresh_drawing();
}



void learn_UI_button(GtkWidget */*Widget*/, ezgl::application *application)
{
    application -> update_message("please read the output from terminal for the "
    "user manual for our map" );
    cout << "1. Please use zoom bottom to zoom in or out " <<endl;
    cout <<"2. For the Highlight Street bottom, please only input the correct "
            "input with first letter in every word capitalized " << endl;
    cout << "3. you can swtich the theme colour between day mode or night mode "
            "by our Day Mode bottom and Night Mode bottom" << endl ;
    cout << "4. you can find the intersection between two street by using our "
            "Find bottom" << endl;
    cout << "   by clicking the Find bottom first, then type in the two street "
            "name into the terminal" << endl;
    cout << "5. you can navigate left, right, up and down by using the  left, "
            "right, up and down bottoms" << endl;
    cout << endl;
    cout << "6. you can find the best path between two intersection by using "
            "either of our three find path bottoms" << endl;
    cout << "find walk & drive path by search bar: type input into search bar "
            "as " << endl;
    cout << "instructed on GUI, then type the walking speed and time limit into "
            "terminal" << endl;
    cout << "----------------------------------------------------------------"
            "-----------" << endl;
    cout << "find path by using the search bar: type input into search bar "
            "as " << endl;
    cout << "instructed on GUI, then click the bottom" << endl;
    cout << "---------------------------------------------------------------"
            "------------" << endl;
    cout << "find path by click at 2 locations: click at two locations at the"
            " map " << endl;
    cout << "it will show the path in orange" << endl;
    cout << "----------------------------------------------------------------"
            "-----------" << endl;
    application->refresh_drawing();
}

void test_FIND_button(GtkWidget */*Widget*/, ezgl::application *application)
{
  // application -> update_message("Find button pressed");
   //application -> refresh_drawing();
         GtkEntry* text_entry = (GtkEntry *) application->get_object("TextInput");
    const char* text = gtk_entry_get_text(text_entry);
    

    application->update_message(text);
    existing_street_name = text;
    
    application->refresh_drawing();
}

void test_night_mode_button(GtkWidget */*Widget*/, ezgl::application *application)
{
   application -> update_message("Night mode button pressed");
   open_night_mode = true;
   application -> refresh_drawing();
}

void pathing_test(GtkWidget */*Widget*/, ezgl::application *application)
{   IntersectionIndex initial;
    IntersectionIndex final;
        cout << " Intersection 1:";
        cin >> initial;
    cin.ignore(1000, '\n');
    cout << endl;
        cout << " Intersection 2:";
        cin >> final;
    cin.ignore(1000, '\n');
    
            vector<StreetSegmentIndex> path;
            path = find_path_between_intersections(initial,final,15);    
            
   application -> refresh_drawing();
}

void findIntByID(GtkWidget */*Widget*/, ezgl::application *application)
{   IntersectionIndex initial;
        cout << " Intersection:";
        cin >> initial;
    cin.ignore(1000, '\n');
   flag_for_found_intersection_by_find_bottom = initial;
            
   application -> refresh_drawing();
}


void test_day_mode_button(GtkWidget */*Widget*/, ezgl::application *application)
{
   application -> update_message("day mode button pressed");
   open_night_mode = false;
   application -> refresh_drawing();
}

//Function to determine width of streets based on zoom level
double streetWidthCheck (ezgl::renderer *g, double base) {
    ezgl::rectangle currentBounds = g->get_visible_world();
    if ((currentBounds.right() - currentBounds.left()) < 0.0008)
        {
        return base *60;
    }
    else if  ((currentBounds.right() - currentBounds.left()) < 0.002)
        {
        return base*20;
    }
    else if  ((currentBounds.right() - currentBounds.left()) < 0.004)
        {
        return base*16;
    }
    else if ((currentBounds.right() - currentBounds.left()) < 0.008) {
        return base*10;
        }
    else if ((currentBounds.right() - currentBounds.left()) < 0.022) {
        return base*6;
        }
    else if ((currentBounds.right() - currentBounds.left()) < 0.05) {
        return base*3;
        }
    return base;
}

void act_on_mouse_move(ezgl::application */*application*/, GdkEventButton 
*/*event*/, double x, double y)
{
  //std::cout << "Mouse move at coordinates (" << x << "," << y << ") "<< std::endl;
}

void act_on_key_press(ezgl::application *application, GdkEventKey */*event*/, char *key_name)
{
  //application->update_message("Key Pressed");

  //std::cout << key_name <<" key is pressed" << std::endl;
}


void drawPOI_Name(ezgl::renderer *g) {
    g->set_text_rotation(0);
    g->set_font_size(10);
    if(open_night_mode == true) {
        g->set_color(ezgl::WHITE);
    }
    else {
        g->set_color(ezgl::BLACK);
    }
    int total_POI = getNumPointsOfInterest();
    for (int i = 0; i < total_POI; i++) {
        string POI_name = getPointOfInterestName(i);
        LatLon POI_position = getPointOfInterestPosition(i);
        double POI_x = x_from_lon(POI_position.lon());
        double POI_y = y_from_lat(POI_position.lat());
        g->draw_text({POI_x,POI_y},  POI_name, 10, 10); //Rough text size, 
        //maybe I can make this dynamic?
    }
}



bool highwayCheck(InfoStreetSegment segment) {
    if (segment.speedLimit >= 100) {
        return true;
    }
    else {
        return false;
    }       
}

bool biggestRoadCheck(InfoStreetSegment segment){
    if (segment.speedLimit >= 75 && segment.speedLimit <= 100) {
        return true;
    }
    else {
        return false;
    }
}

bool oneWayCheck(InfoStreetSegment segment){
    
    if (segment.oneWay == true) {
        return true;
    }
    else {
        return false;
    }
}

ezgl::color if_background_darker(){//for swith between dark mode and day mode 
    ezgl::color background_light(246, 242, 242, 255);
    //ezgl::color background_dark
    if(open_night_mode == false){
        return background_light;
    }else if (open_night_mode == true) {return ezgl::BLACK;}
    //return ezgl::BLACK;
}

