/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   city.h
 * Author: lisharo5
 *
 * Created on February 1, 2020, 11:53 AM
 */

#ifndef CITY_H
#define CITY_H
#include "street.h"
#include <vector>
using namespace std;

class city {
public:
    vector<street> allStreets;
    
    city();
    city(const city& orig);
    virtual ~city();
private:

};

#endif /* CITY_H */

