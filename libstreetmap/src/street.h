/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   street.h
 * Author: lisharo5
 *
 * Created on February 1, 2020, 11:54 AM
 */

#ifndef STREET_H
#define STREET_H
using namespace std;
#include <vector>

using namespace std;

class street {
public:
    vector<int> allIntersections;
    vector<int> allStreetSegments;
    street();
    street(const street& orig);
    virtual ~street();
private:

};

#endif /* STREET_H */

