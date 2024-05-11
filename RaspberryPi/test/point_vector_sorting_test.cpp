#include <algorithm>
#include <opencv2/opencv.hpp>

// Custom comparator function for sorting cv::Point objects
bool comparePoints(const cv::Point &a, const cv::Point &b) {
  if (a.x == b.x) {
    return a.y < b.y;
  }
  return a.x < b.x;
}

int main() {
  std::vector<cv::Point> points = {cv::Point(3, 2), cv::Point(1, 5),
                                   cv::Point(4, 3)};

  // Sorting the vector of cv::Point using the custom comparator function
  std::sort(points.begin(), points.end(), comparePoints);

  // Output the sorted points
  for (const auto &point : points) {
    std::cout << "Point (" << point.x << ", " << point.y << ")" << std::endl;
  }

  return 0;
}
