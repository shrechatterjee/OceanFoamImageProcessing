/*	skc105@ecs.soton.ac.uk */

#include "whitecap.h"

Whitecap::Whitecap(vector<Point> new_contour)
{
    //ctor
    contour = new vector<Point>;
    centroid = new Point;
    mask = new Mat(2048, 2048, CV_8UC1, Scalar::all(0));

    setContour(new_contour);
}

Whitecap::~Whitecap()
{
    delete contour;
    delete centroid;
    delete mask;
}

Mat Whitecap::getMask()
{
    return mask->clone();
}

vector<Point> Whitecap::getContour()
{
    return *contour;
}

Point Whitecap::getCentroid()
{
    return *centroid;
}

int Whitecap::getSize()
{
    return total_size;
}

int Whitecap::getIntensity()
{
    return intensity;
}

void Whitecap::setContour(vector<Point> new_contour)
{
    *contour = new_contour;
    calcCentroid();
    calcSize();
    calcMask();
    calcIntensity();
    return;
}

void Whitecap::calcCentroid()
{
    Moments mu = moments(*contour, false);
    *centroid = Point(mu.m10/mu.m00 , mu.m01/mu.m00);
    return;
}

void Whitecap::calcMask()
{
    vector<vector<Point> > contour_vec;
    contour_vec.push_back(*contour);
    drawContours(*mask, contour_vec, -1, Scalar(255), -1, 8, noArray(), INT_MAX, Point());
    return;
}

void Whitecap::calcSize()
{
    total_size = contourArea(*contour, false);
    return;
}

void Whitecap::calcIntensity()
{
    return;
}
