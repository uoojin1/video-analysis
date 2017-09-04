//
//  main.cpp
//  OpenCV tutorial
//
//  Created by EugeneMoon on 2017. 8. 16..
//  Copyright © 2017년 EugeneMoon. All rights reserved.
//

#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgcodecs.hpp"
#include <cmath>
#define PI 3.14159265

using namespace cv;
using namespace std;

int frameNumber;
int maxFrame;
int ballSize;
int frameWidth;
int frameHeight;
int counter=0;

Vec3f hsv;

Point start;
Point finish;

Point prevPoint;
Point currentPoint;

Point prevPinPoint;
Point pinPoint;


#define w 400

//image processing variables -- can be altered by using the trackbars
int H_MIN = 0;
int H_MAX = 256;
int S_MIN = 0;
int S_MAX = 256;
int V_MIN = 0;
int V_MAX = 256;

//default capture width and height
const int FRAME_WIDTH = 640;
const int FRAME_HEIGHT = 480;

//max number of objects to be detected in frame
const int MAX_NUM_OBJECTS=50;

//minimum and maximum object area
const int MIN_OBJECT_AREA = 20*20;
const int MAX_OBJECT_AREA = 2350;//FRAME_HEIGHT*FRAME_WIDTH/1.5;

//names that will appear at the top of each window
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";

/**
 Function prototypes
 */
double rotateHue(int degress);
void HSVtoRGB(float& fR, float& fG, float& fB, float& fH, float& fS, float& fV);
void on_trackbar( int, void* );
string intToString(int number);
void createTrackbars();
void drawObject(int x, int y,Mat &frame);
void morphOps(Mat &thresh);
double angleBetweenTwoPoints(Point p1, Point p2);
double distanceBetweenTwoPoints(double x1, double y1, double x2, double y2);
bool jump(Point a, Point b);
Point trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed);
void MyLine( Mat img, Point start, Point end );
void MyFilledCircle( Mat img, Point center, int frameNumber );
void MyEllipse( Mat img, double angle );


int main(int argc, char* argv[])
{
    frameNumber = 0;
    Point pinPoint;
    bool trackObjects = true;
    bool useMorphOps = true;
    Mat cameraFeed;
    Mat HSV;
    Mat threshold;
    Mat trace;
    Mat resized;

    int x=0, y=0;
    hsv[1]=0;
    hsv[2]=1;
    double prevLineDistance=0;
    double lineDistance=0;

    createTrackbars();
    VideoCapture capture;
    capture.open("mouse2.mp4");
    
    if(!capture.isOpened()){
        cout<<"ERROR ACQUIRING VIDEO FEED\n";
        getchar();
        return -1;
    }else{
        frameWidth = capture.get(CV_CAP_PROP_FRAME_WIDTH);
        frameHeight = capture.get(CV_CAP_PROP_FRAME_HEIGHT);
        cout << frameWidth << endl;
        cout << frameHeight << endl;
    }
    capture.set(CV_CAP_PROP_FRAME_WIDTH,FRAME_WIDTH);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT,FRAME_HEIGHT);

    char tracking_window[] = "Tracking Path";
    Mat tracking_image = Mat::zeros( frameHeight, frameWidth, CV_8UC3 );
    maxFrame = capture.get(CV_CAP_PROP_FRAME_COUNT)-10;
    
    
    createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar );
    createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar );
    createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar );
    createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar );
    createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar );
    createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar );
    

    
    while(capture.get(CV_CAP_PROP_POS_FRAMES)<capture.get(CV_CAP_PROP_FRAME_COUNT)-10){
        capture.read(cameraFeed);
        cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
        //inRange(HSV,Scalar(0,0,0),Scalar(54,5,256),threshold);
        inRange(HSV,Scalar(0,0,0),Scalar(H_MAX,S_MAX,V_MAX),threshold);
        if(useMorphOps)
            morphOps(threshold);
        if(trackObjects){
            pinPoint = trackFilteredObject(x,y,threshold,cameraFeed);
            pinPoint.x = floor(pinPoint.x * 800/1920);
            pinPoint.y = floor(pinPoint.y * 600/1080);
        }
        //cout << pinPoint << endl;
        
        prevPoint = currentPoint;
        currentPoint = pinPoint;
        
        if(counter == 0){
            start.x = pinPoint.x;
            start.y = pinPoint.y;
            prevPoint.x = start.x;
            prevPoint.y = start.y;
            currentPoint.x = start.x;
            currentPoint.y = start.y;
        }
        if(counter == capture.get(CV_CAP_PROP_FRAME_COUNT)-15){
            finish.x = pinPoint.x;
            finish.y = pinPoint.y;
        }
        trace.copySize(threshold);
        resize(threshold,resized,Size(FRAME_WIDTH,FRAME_HEIGHT));
        imshow(windowName2,resized); // this will show the threshold image
        moveWindow(windowName2, 640, 0);
        
        int dist = distanceBetweenTwoPoints(currentPoint.x, currentPoint.y, prevPoint.x, prevPoint.y);
        prevLineDistance = lineDistance;
        lineDistance = distanceBetweenTwoPoints(currentPoint.x, currentPoint.y, prevPoint.x, prevPoint.y);
        
        if(dist<=32 && dist>=-32){
            MyLine(tracking_image, prevPoint, currentPoint);
        }else{
            currentPoint = prevPoint;
        }
        putText(tracking_image, "S", start, 1, 1, Scalar(0,0,255),2);
        resize(tracking_image,resized,Size(FRAME_WIDTH,FRAME_HEIGHT));
        imshow( tracking_window, resized);
        //moveWindow("Tracking Path", 800, 1700);
        
        resize(cameraFeed,resized,Size(FRAME_WIDTH,FRAME_HEIGHT));
        imshow(windowName,resized);
        //moveWindow("Tracking Path", 1800, 960);


        waitKey(30);
        if(counter%3==0){
            frameNumber++;
        }
        
        counter++;
    }
    putText(tracking_image, "F", finish, 1, 1, Scalar(0,0,255),2);
    imwrite("mouse_path.jpg", tracking_image);
    return 0;
}

/**
 Functions
 */

double rotateHue(int degrees){
    return degrees%360;
}

void HSVtoRGB(float& fR, float& fG, float& fB, float& fH, float& fS, float& fV) {
    float fC = fV * fS; // Chroma
    float fHPrime = fmod(fH / 60.0, 6);
    float fX = fC * (1 - fabs(fmod(fHPrime, 2) - 1));
    float fM = fV - fC;
    
    if(0 <= fHPrime && fHPrime < 1) {
        fR = fC;
        fG = fX;
        fB = 0;
    } else if(1 <= fHPrime && fHPrime < 2) {
        fR = fX;
        fG = fC;
        fB = 0;
    } else if(2 <= fHPrime && fHPrime < 3) {
        fR = 0;
        fG = fC;
        fB = fX;
    } else if(3 <= fHPrime && fHPrime < 4) {
        fR = 0;
        fG = fX;
        fB = fC;
    } else if(4 <= fHPrime && fHPrime < 5) {
        fR = fX;
        fG = 0;
        fB = fC;
    } else if(5 <= fHPrime && fHPrime < 6) {
        fR = fC;
        fG = 0;
        fB = fX;
    } else {
        fR = 0;
        fG = 0;
        fB = 0;
    }
    
    fR += fM;
    fG += fM;
    fB += fM;
    
    fR*=255;
    fG*=255;
    fB*=255;
}

void on_trackbar( int, void* )
{//This function gets called whenever a
    // trackbar position is changed
    cout << "S_max: " << S_MAX << endl;
    cout << "H_max: " << H_MAX << endl;
    cout << "S_min: " <<S_MIN << endl;
    cout << "V_max: " <<V_MAX << endl;
    cout << "H_min: " <<H_MIN << endl;
    cout << "V_min: " <<V_MIN << endl;
    cout << endl;
}
string intToString(int number){
    
    std::stringstream ss;
    ss << number;
    return ss.str();
}
void createTrackbars(){
    
    namedWindow(trackbarWindowName,0);
    moveWindow(trackbarWindowName, 2000, 0);
    //create trackbars and insert them into window
    //3 parameters are: the address of the variable that is changing when the trackbar is moved(eg.H_LOW),
    //the max value the trackbar can move (eg. H_HIGH),
    //and the function that is called whenever the trackbar is moved(eg. on_trackbar)
    //                                  ---->    ---->     ---->
    createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, H_MAX, on_trackbar );
    createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, H_MAX, on_trackbar );
    createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, S_MAX, on_trackbar );
    createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, S_MAX, on_trackbar );
    createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, V_MAX, on_trackbar );
    createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, V_MAX, on_trackbar );
}
void drawObject(int x, int y,Mat &frame){
    
    circle(frame,Point(x,y),20,Scalar(0,255,0),2);
    if(y-25>0)
        line(frame,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,0),Scalar(0,255,0),2);
    if(y+25<FRAME_HEIGHT)
        line(frame,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(x,FRAME_HEIGHT),Scalar(0,255,0),2);
    if(x-25>0)
        line(frame,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(0,y),Scalar(0,255,0),2);
    if(x+25<FRAME_WIDTH)
        line(frame,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
    else line(frame,Point(x,y),Point(FRAME_WIDTH,y),Scalar(0,255,0),2);
    
    putText(frame,intToString(x)+","+intToString(y),Point(x,y+30),1,1,Scalar(0,255,0),2);
    
}
void morphOps(Mat &thresh){
    
    //create structuring element that will be used to "dilate" and "erode" image.
    //the element chosen here is a 3px by 3px rectangle
    
    Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
    //dilate with larger element so make sure object is nicely visible
    Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));
    
    erode(thresh,thresh,erodeElement);
    erode(thresh,thresh,erodeElement);
    
    dilate(thresh,thresh,dilateElement);
    dilate(thresh,thresh,dilateElement);
    
}

double angleBetweenTwoPoints(Point p1, Point p2){
    if(p2.x - p1.x == 0){
        return 0;
    }
    double angle = (p2.y - p1.y)/(p2.x - p1.x);
    return angle;
}

double distanceBetweenTwoPoints(double x1, double y1, double x2, double y2){
    //  cout << "distanceBetweenTwoPoints: " << x2-x1 + y2-y1 << endl;
    return x2-x1 + y2-y1;
}

bool jump(Point a, Point b){
    //cout << "distance is" << sqrt(abs(a.x-b.x)*abs(a.x-b.x)+abs(a.y-b.y)*abs(a.y-b.y)) << endl;
    int distance = distanceBetweenTwoPoints(a.x, a.y, b.x, b.y);
    if(distance>800 || distance<-800){
        return false;
    }
    if(distance>40 || distance<-40){
        return true;
    }
    return false;
}

Point trackFilteredObject(int &x, int &y, Mat threshold, Mat &cameraFeed){
    
    Mat temp;
    threshold.copyTo(temp);
    //these two vectors needed for output of findContours
    vector< vector<Point> > contours;
    vector<Vec4i> hierarchy;
    //find contours of filtered image using openCV findContours function
    findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
    //use moments method to find our filtered object
    double refArea = 0;
    bool objectFound = false;
    if (hierarchy.size() > 0) {
        int numObjects = (int)hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects<MAX_NUM_OBJECTS){
            for (int index = 0; index >= 0; index = hierarchy[index][0]) {
                
                Moments moment = moments((cv::Mat)contours[index]);
                double area = moment.m00;
                
                //if the area is less than 20 px by 20px then it is probably just noise
                //if the area is the same as the 3/2 of the image size, probably just a bad filter
                //we only want the object with the largest area so we safe a reference area each
                //iteration and compare it to the area in the next iteration.
                if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
                    x = moment.m10/area;
                    y = moment.m01/area;
                    objectFound = true;
                    refArea = area;
                    
                    prevPinPoint = pinPoint;
                    pinPoint = Point(x,y);
                    
                    if(jump(pinPoint,prevPinPoint)){
                       // cout << "jump " << endl;
                        objectFound = false;
                        pinPoint = prevPinPoint;
                        //return Point(0,0);
                        return prevPinPoint;
                    }
                }else{
                    objectFound = false;
                    return prevPinPoint;
                }
                
                
            }
            //let user know you found an object
            if(objectFound ==true){
                putText(cameraFeed,"Tracking Object",Point(0,50),2,1,Scalar(0,255,0),2);
                //draw object location on screen
                drawObject(x,y,cameraFeed);
            }else{
                return Point(100,100);
            }
            
        }else putText(cameraFeed,"TOO MUCH NOISE! ADJUST FILTER",Point(0,50),1,2,Scalar(0,0,255),2);
    }
    prevPinPoint = pinPoint;
    return pinPoint;
}

void MyLine( Mat img, Point start, Point end )
{
    hsv[0]=rotateHue(frameNumber%360);
    hsv[2]=1;
    Vec3f bgr;
    if(counter<10){
        hsv[1]+=0.1000000000;
    }
    if(counter >= maxFrame-100){
        hsv[1]-=0.01000000000;
    }
    HSVtoRGB(bgr[2], bgr[1], bgr[0], hsv[0], hsv[1], hsv[2]);
    //    cout <<"H: "<<hsv[0]<<" S: "<<hsv[1]<<" V: "<<hsv[2]<<endl;
    //    cout <<"B: "<<bgr[0]<<" G: "<<bgr[1]<<" R: "<<bgr[2]<<endl;
    
    int thickness = 0;
    int lineType = LINE_8;
    int dist = start.x - end.x + start.y - end.y;
    if(dist>-100 && dist<100){
        // cout << "distance inside the line method: " << dist << endl;
        line( img,
             start,
             end,
             bgr,
             thickness,
             lineType );
    }
}

void MyFilledCircle( Mat img, Point center, int frameNumber )
{
    hsv[0]=rotateHue(frameNumber%360);
    hsv[2]=1;
    Vec3f bgr;
    if(counter<100){
        hsv[1]+=0.01000000000;
    }
    if(counter >= maxFrame-100){
        hsv[1]-=0.01000000000;
    }
    HSVtoRGB(bgr[2], bgr[1], bgr[0], hsv[0], hsv[1], hsv[2]);
    //    cout <<"H: "<<hsv[0]<<" S: "<<hsv[1]<<" V: "<<hsv[2]<<endl;
    //    cout <<"B: "<<bgr[0]<<" G: "<<bgr[1]<<" R: "<<bgr[2]<<endl;
    
    if(frameNumber==0 || frameNumber==maxFrame){
        ballSize = 7;
    }
    else if (ballSize>0 && frameNumber%7==0 && counter < 100){
        ballSize--;
    }
    else if (ballSize<7 && counter>maxFrame-100){
        if(counter%7==0){
            ballSize++;
        }
    }
    circle( img,
           center,
           ballSize,
           bgr,
           FILLED,
           LINE_8 );
}

void MyEllipse( Mat img, double angle )
{
    int thickness = 2;
    int lineType = 8;
    
    ellipse( img,
            Point( w/2, w/2 ),
            Size( w/4, w/16 ),
            angle,
            0,
            360,
            Scalar( 255, 0, 0 ),
            thickness,
            lineType );
}
//![myellipse]



//#define w 400
//
//using namespace cv;
//
///// Function headers
//void MyEllipse( Mat img, double angle );
//void MyFilledCircle( Mat img, Point center );
//void MyPolygon( Mat img );
//void MyLine( Mat img, Point start, Point end );
//
///**
// * @function main
// * @brief Main function
// */
//int main( void ){
//    
//    //![create_images]
//    /// Windows names
//    char atom_window[] = "Drawing 1: Atom";
//    char rook_window[] = "Drawing 2: Rook";
//    
//    /// Create black empty images
//    Mat tracking_image = Mat::zeros( w, w, CV_8UC3 );
//    Mat rook_image = Mat::zeros( w, w, CV_8UC3 );
//    //![create_images]
//    
//    /// 1. Draw a simple atom:
//    /// -----------------------
//    
//    //![draw_atom]
//    /// 1.a. Creating ellipses
//    MyEllipse( tracking_image, 90 );
//    MyEllipse( tracking_image, 0 );
//    MyEllipse( tracking_image, 45 );
//    MyEllipse( tracking_image, -45 );
//    
//    /// 1.b. Creating circles
//    MyFilledCircle( tracking_image, Point( w/2, w/2) );
//    //![draw_atom]
//    
//    /// 2. Draw a rook
//    /// ------------------
//    
//    //![draw_rook]
//    /// 2.a. Create a convex polygon
//    MyPolygon( rook_image );
//    
//    //![rectangle]
//    /// 2.b. Creating rectangles
//    rectangle( rook_image,
//              Point( 0, 7*w/8 ),
//              Point( w, w),
//              Scalar( 0, 255, 255 ),
//              FILLED,
//              LINE_8 );
//    //![rectangle]
//    
//    /// 2.c. Create a few lines
//    MyLine( rook_image, Point( 0, 15*w/16 ), Point( w, 15*w/16 ) );
//    MyLine( rook_image, Point( w/4, 7*w/8 ), Point( w/4, w ) );
//    MyLine( rook_image, Point( w/2, 7*w/8 ), Point( w/2, w ) );
//    MyLine( rook_image, Point( 3*w/4, 7*w/8 ), Point( 3*w/4, w ) );
//    //![draw_rook]
//    
//    /// 3. Display your stuff!
//    imshow( atom_window, tracking_image );
//    moveWindow( atom_window, 0, 200 );
//    imshow( rook_window, rook_image );
//    moveWindow( rook_window, w, 200 );
//    
//    waitKey( 0 );
//    return(0);
//}
//
///// Function Declaration
//
////![myellipse]
///**
// * @function MyEllipse
// * @brief Draw a fixed-size ellipse with different angles
// */

//
////![myfilledcircle]
///**
// * @function MyFilledCircle
// * @brief Draw a fixed-size filled circle
// */
//void MyFilledCircle( Mat img, Point center )
//{
//    circle( img,
//           center,
//           w/32,
//           Scalar( 0, 0, 255 ),
//           FILLED,
//           LINE_8 );
//}
////![myfilledcircle]
//
////![mypolygon]
///**
// * @function MyPolygon
// * @brief Draw a simple concave polygon (rook)
// */
//void MyPolygon( Mat img )
//{
//    int lineType = LINE_8;
//    
//    /** Create some points */
//    Point rook_points[1][20];
//    rook_points[0][0]  = Point(    w/4,   7*w/8 );
//    rook_points[0][1]  = Point(  3*w/4,   7*w/8 );
//    rook_points[0][2]  = Point(  3*w/4,  13*w/16 );
//    rook_points[0][3]  = Point( 11*w/16, 13*w/16 );
//    rook_points[0][4]  = Point( 19*w/32,  3*w/8 );
//    rook_points[0][5]  = Point(  3*w/4,   3*w/8 );
//    rook_points[0][6]  = Point(  3*w/4,     w/8 );
//    rook_points[0][7]  = Point( 26*w/40,    w/8 );
//    rook_points[0][8]  = Point( 26*w/40,    w/4 );
//    rook_points[0][9]  = Point( 22*w/40,    w/4 );
//    rook_points[0][10] = Point( 22*w/40,    w/8 );
//    rook_points[0][11] = Point( 18*w/40,    w/8 );
//    rook_points[0][12] = Point( 18*w/40,    w/4 );
//    rook_points[0][13] = Point( 14*w/40,    w/4 );
//    rook_points[0][14] = Point( 14*w/40,    w/8 );
//    rook_points[0][15] = Point(    w/4,     w/8 );
//    rook_points[0][16] = Point(    w/4,   3*w/8 );
//    rook_points[0][17] = Point( 13*w/32,  3*w/8 );
//    rook_points[0][18] = Point(  5*w/16, 13*w/16 );
//    rook_points[0][19] = Point(    w/4,  13*w/16 );
//    
//    const Point* ppt[1] = { rook_points[0] };
//    int npt[] = { 20 };
//    
//    fillPoly( img,
//             ppt,
//             npt,
//             1,
//             Scalar( 255, 255, 255 ),
//             lineType );
//}
////![mypolygon]
//
////![myline]
///**
// * @function MyLine
// * @brief Draw a simple line
// */

////![myline]






/* ------------ This part is movement tracking --------------
 
 //our sensitivity value to be used in the absdiff() function
 const static int SENSITIVITY_VALUE = 30;
 //size of blur used to smooth the intensity image output from absdiff() function
 const static int BLUR_SIZE = 5;
 //we'll have just one object to search for
 //and keep track of its position.
 int theObject[2] = {0,0};
 //bounding rectangle of the object, we will use the center of this as its position.
 Rect objectBoundingRectangle = Rect(0,0,0,0);
 
 
 //int to string helper function
 string intToString(int number){
 
 //this function has a number input and string output
 std::stringstream ss;
 ss << number;
 return ss.str();
 }
 
 // Function to convert octal number to decimal
 int octalToDecimal(int octalNumber)
 {
 int decimalNumber = 0, i = 0, rem;
 while (octalNumber != 0)
 {
 rem = octalNumber % 10;
 octalNumber /= 10;
 decimalNumber += rem * pow(8, i);
 ++i;
 }
 return decimalNumber;
 }
 
 void searchForMovement(Mat thresholdImage, Mat &cameraFeed){
 //notice how we use the '&' operator for objectDetected and cameraFeed. This is because we wish
 //to take the values passed into the function and manipulate them, rather than just working with a copy.
 //eg. we draw to the cameraFeed to be displayed in the main() function.
 bool objectDetected = false;
 Mat temp;
 thresholdImage.copyTo(temp);
 //these two vectors needed for output of findContours
 vector< vector<Point> > contours;
 vector<Vec4i> hierarchy;
 //find contours of filtered image using openCV findContours function
 //findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );// retrieves all contours
 findContours(temp,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE );// retrieves external contours
 
 //if contours vector is not empty, we have found some moving objects
 if(contours.size()>0)objectDetected=true;
 else objectDetected = false;
 
 if(objectDetected){
 //the largest contour is found at the end of the contours vector
 //we will simply assume that the biggest contour is the object we are looking for.
 vector< vector<Point> > largestContourVec;
 largestContourVec.push_back(contours.at(contours.size()-1));
 //make a bounding rectangle around the largest contour then find its centroid
 //this will be the object's final estimated position.
 objectBoundingRectangle = boundingRect(largestContourVec.at(0));
 int xpos = objectBoundingRectangle.x+objectBoundingRectangle.width/2;
 int ypos = objectBoundingRectangle.y+objectBoundingRectangle.height/2;
 
 Mat regionOfInterest = temp(Rect(objectBoundingRectangle.x,
 objectBoundingRectangle.y,
 objectBoundingRectangle.width,
 objectBoundingRectangle.height));
 //  imshow("RegionOfInterest",regionOfInterest);
 // waitKey(1);
 
 
 int x_accumulate = 0;
 int y_accumulate = 0;
 
 for(int i=0; i<regionOfInterest.rows; i++){
 for(int j=0; j<regionOfInterest.cols; j++){
 //  std::cout << "Matrix of image loaded is: " << octalToDecimal(regionOfInterest.at<uchar>(i,j));
 if(regionOfInterest.at<uchar>(i,j)>0){
 y_accumulate += j;
 x_accumulate += i;
 }
 }
 }
 //        std::cout << "ROI max row is: " << regionOfInterest.rows << endl;
 //        std::cout << "ROI max col is: " << regionOfInterest.cols << endl;
 //        std::cout << "average x value is: " << x_accumulate/regionOfInterest.rows/regionOfInterest.cols << endl;
 //        std::cout << "average y value is: " << y_accumulate/regionOfInterest.rows/regionOfInterest.cols << endl;
 
 xpos = objectBoundingRectangle.x + x_accumulate/regionOfInterest.rows/regionOfInterest.cols;
 ypos = objectBoundingRectangle.y + y_accumulate/regionOfInterest.rows/regionOfInterest.cols;
 
 //update the objects positions by changing the 'theObject' array values
 theObject[0] = xpos , theObject[1] = ypos;
 }
 //make some temp x and y variables so we dont have to type out so much
 int x = theObject[0];
 int y = theObject[1];
 
 //draw some crosshairs around the object
 circle(cameraFeed,Point(x,y),20,Scalar(0,255,0),2);
 line(cameraFeed,Point(x,y),Point(x,y-25),Scalar(0,255,0),2);
 line(cameraFeed,Point(x,y),Point(x,y+25),Scalar(0,255,0),2);
 line(cameraFeed,Point(x,y),Point(x-25,y),Scalar(0,255,0),2);
 line(cameraFeed,Point(x,y),Point(x+25,y),Scalar(0,255,0),2);
 
 //write the position of the object to the screen
 putText(cameraFeed,"Tracking object at (" + intToString(x)+","+intToString(y)+")",Point(x,y),1,1,Scalar(255,0,0),2);
 
 }
 
 #include "opencv2/opencv.hpp"
 
 using namespace cv;
 //
 //int main(){
 //
 //    // Create a VideoCapture object and open the input file
 //    // If the input is the web camera, pass 0 instead of the video file name
 //    VideoCapture cap("mouse1.avi");
 //
 //    // Check if camera opened successfully
 //    if(!cap.isOpened()){
 //        cout << "Error opening video stream or file" << endl;
 //        return -1;
 //    }
 //
 //    while(1){
 //
 //        Mat frame;
 //        // Capture frame-by-frame
 //        cap >> frame;
 //
 //        // If the frame is empty, break immediately
 //        if (frame.empty())
 //            break;
 //
 //        // Display the resulting frame
 //        imshow( "Frame", frame );
 //
 //        // Press  ESC on keyboard to exit
 //        char c=(char)waitKey(25);
 //        if(c==27)
 //            break;
 //    }
 //
 //    // When everything done, release the video capture object
 //    cap.release();
 //
 //    // Closes all the frames
 //    destroyAllWindows();
 //
 //    return 0;
 //}
 
 int main(){
 
 //some boolean variables for added functionality
 bool objectDetected = false;
 //these two can be toggled by pressing 'd' or 't'
 bool debugMode = false;
 bool trackingEnabled = true;
 //pause and resume code
 bool pause = false;
 //set up the matrices that we will need
 //the two frames we will be comparing
 Mat frame1,frame2;
 //their grayscale images (needed for absdiff() function)
 Mat grayImage1,grayImage2;
 //resulting difference image
 Mat differenceImage;
 //thresholded difference image (for use in findContours() function)
 Mat thresholdImage;
 //video capture object.
 VideoCapture capture;
 
 while(1){
 
 //we can loop the video by re-opening the capture every time the video reaches its last frame
 
 capture.open("mouse1.avi");
 if(!capture.isOpened()){
 cout<<"ERROR ACQUIRING VIDEO FEED\n";
 getchar();
 return -1;
 }
 
 //check if the video has reach its last frame.
 //we add '-1' because we are reading two frames from the video at a time.
 //if this is not included, we get a memory error!
 
 while(capture.get(CV_CAP_PROP_POS_FRAMES)<capture.get(CV_CAP_PROP_FRAME_COUNT)-1){
 //read first frame
 capture.read(frame1);
 //convert frame1 to gray scale for frame differencing
 cv::cvtColor(frame1,grayImage1,COLOR_BGR2GRAY);
 //copy second frame
 capture.read(frame2);
 //convert frame2 to gray scale for frame differencing
 cv::cvtColor(frame2,grayImage2,COLOR_BGR2GRAY);
 //perform frame differencing with the sequential images. This will output an "intensity image"
 //do not confuse this with a threshold image, we will need to perform thresholding afterwards.
 cv::absdiff(grayImage1,grayImage2,differenceImage);
 //threshold intensity image at a given sensitivity value
 cv::threshold(differenceImage,thresholdImage,SENSITIVITY_VALUE,255,THRESH_BINARY);
 if(debugMode==true){
 //show the difference image and threshold image
 cv::imshow("Difference Image",differenceImage);
 waitKey(1);
 cv::imshow("Threshold Image", thresholdImage);
 waitKey(1);
 }else{
 //if not in debug mode, destroy the windows so we don't see them anymore
 cv::destroyWindow("Difference Image");
 cv::destroyWindow("Threshold Image");
 }
 //blur the image to get rid of the noise. This will output an intensity image
 cv::blur(thresholdImage,thresholdImage,cv::Size(BLUR_SIZE,BLUR_SIZE));
 //threshold again to obtain binary image from blur output
 cv::threshold(thresholdImage,thresholdImage,SENSITIVITY_VALUE,255,THRESH_BINARY);
 if(debugMode==false){
 //show the threshold image after it's been "blurred"
 imshow("Final Threshold Image",thresholdImage);
 waitKey(1);
 }
 else {
 //if not in debug mode, destroy the windows so we don't see them anymore
 cv::destroyWindow("Final Threshold Image");
 }
 
 //if tracking enabled, search for contours in our thresholded image
 if(trackingEnabled){
 searchForMovement(thresholdImage,frame1);
 }
 
 //show our captured frame
 imshow("Frame1",frame1);
 waitKey(1);
 //check to see if a button has been pressed.
 //this 10ms delay is necessary for proper operation of this program
 //if removed, frames will not have enough time to referesh and a blank
 //image will appear.
 switch(waitKey(10)){
 
 case 27: //'esc' key has been pressed, exit program.
 return 0;
 case 116: //'t' has been pressed. this will toggle tracking
 trackingEnabled = !trackingEnabled;
 if(trackingEnabled == false) cout<<"Tracking disabled."<<endl;
 else cout<<"Tracking enabled."<<endl;
 break;
 case 100: //'d' has been pressed. this will debug mode
 debugMode = !debugMode;
 if(debugMode == false) cout<<"Debug mode disabled."<<endl;
 else cout<<"Debug mode enabled."<<endl;
 break;
 case 112: //'p' has been pressed. this will pause/resume the code.
 pause = !pause;
 if(pause == true){ cout<<"Code paused, press 'p' again to resume"<<endl;
 while (pause == true){
 //stay in this loop until
 switch (waitKey()){
 //a switch statement inside a switch statement? Mind blown.
 case 112:
 //change pause back to false
 pause = false;
 cout<<"Code Resumed"<<endl;
 break;
 }
 }
 }
 
 
 
 }
 }
 //release the capture before re-opening and looping again.
 capture.release();
 }
 
 return 0;
 
 }
 ----------------------------    end of movement tracking ---------------------------*/
