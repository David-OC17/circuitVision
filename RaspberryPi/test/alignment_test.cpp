#include <opencv2/opencv.hpp>
#include "opencv2/features2d.hpp"
 
using namespace std;
using namespace cv;
 
const int MAX_FEATURES = 500;
const float GOOD_MATCH_PERCENT = 0.15f;
 
void alignImages(Mat &im1, Mat &im2, Mat &im1Reg, Mat &h)
{
  Mat im1Gray, im2Gray;
  cvtColor(im1, im1Gray, cv::COLOR_BGR2GRAY);
  cvtColor(im2, im2Gray, cv::COLOR_BGR2GRAY);
 
  std::vector<KeyPoint> keypoints1, keypoints2;
  Mat descriptors1, descriptors2;
 
  Ptr<Feature2D> orb = ORB::create(MAX_FEATURES);
  orb->detectAndCompute(im1Gray, Mat(), keypoints1, descriptors1);
  orb->detectAndCompute(im2Gray, Mat(), keypoints2, descriptors2);
 
  std::vector<DMatch> matches;
  Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("BruteForce-Hamming");
  matcher->match(descriptors1, descriptors2, matches, Mat());
 
  std::sort(matches.begin(), matches.end());
 
  const int numGoodMatches = matches.size() * GOOD_MATCH_PERCENT;
  matches.erase(matches.begin()+numGoodMatches, matches.end());
 
  Mat imMatches;
  drawMatches(im1, keypoints1, im2, keypoints2, matches, imMatches);
  imwrite("matches.jpg", imMatches);
 
  std::vector<Point2f> points1, points2;
 
  for( size_t i = 0; i < matches.size(); i++ )
  {
    points1.push_back( keypoints1[ matches[i].queryIdx ].pt );
    points2.push_back( keypoints2[ matches[i].trainIdx ].pt );
  }
 
  h = findHomography( points1, points2, RANSAC );
 
  warpPerspective(im1, im1Reg, h, im2.size());
}
 
int main(int argc, char **argv)
{
  string refFilename("../imgs/test.jpg"); 
  cout << "Reading reference image : " << refFilename << endl; 
  Mat imReference = imread(refFilename);
 
  string imFilename("scanned-form.jpg");
  cout << "Reading image to align : " << imFilename << endl; 
  Mat im = imread(imFilename);
 
  Mat imReg, h;
 
  cout << "Aligning images ..." << endl; 
  alignImages(im, imReference, imReg, h);
 
  string outFilename("aligned.jpg");
  cout << "Saving aligned image : " << outFilename << endl; 
  imwrite(outFilename, imReg);
 
  cout << "Estimated homography : \n" << h << endl; 
}
