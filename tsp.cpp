/**
 *  Timothy Foster
 *  Version: 1.00.150130
 *
 ** This code solves a special case of the Traveling Salesman Problem.  It
 *  was completed for a university project.  Feel free to study, use, or
 *  modify the code for your purposes.
 *
 ** http://publicvar.wikidot.com/post:the-traveling-salesman
 * ****************************************************************************/
 
#include<algorithm>  // sort
#include<cmath>      // sqrt, atan
#include<iostream>
#include<limits>
#include<sstream>
#include<string>
#include<vector>

#define MAX_POINTS 1001
#define SIZE_TO_SEARCH 300

using namespace std;

/*  Class Interface
==============================================================================*/
class PointNode{
public:
    int x;
    int y;
    
    long key;   // used for sorting; distance from origin
    int  index; // which point on list
    
    PointNode();
    PointNode(int x, int y, int index);
    
    float distance_to(const PointNode& other);
};

typedef PointNode*           PointNodePtr;
typedef vector<PointNodePtr> PointList;

struct PointNodeLess{
    bool operator()(PointNodePtr lhs, PointNodePtr rhs){
        return lhs->key < rhs->key;
    }
};

struct PointNodeGreater{
    bool operator()(PointNodePtr lhs, PointNodePtr rhs){
        return lhs->key > rhs->key;
    }
};

/*  Function Interface
==============================================================================*/
void read_points(PointList& points);
inline long distance_squared(const PointNode& p1, const PointNode& p2);
float path_distance(const PointList& path);

PointList find_shortest_path(PointList& points);
void shrink_scope(PointList& points);
PointList segment_method_path(PointList& points, int num_of_segments);
  void fill_segments(PointList& points, vector<PointList>& segments);
  PointList path_from_segments(vector<PointList>& segments);

inline void print_path(const PointList& path);

/*  MAIN
==============================================================================*/
int main(int argc, char** argv){
    PointList points;
    points.reserve(MAX_POINTS);
    read_points(points);
    
    shrink_scope(points);
    
    PointList shortest_path = find_shortest_path(points);
    
    print_path(shortest_path);
    cout << path_distance(shortest_path) << endl;
    
    return 0;
}


/*  Class Definitions
==============================================================================*/
PointNode::PointNode():PointNode(0, 0, -1){}

PointNode::PointNode(int x, int y, int index){
    this->x = x;
    this->y = y;
    this->key = numeric_limits<long>::max();
    this->index = index;
}

float PointNode::distance_to(const PointNode& other){
    long dx = this->x - other.x;
    long dy = this->y - other.y;
    return sqrt(dx*dx + dy*dy);
}

/*  Function Definitions
==============================================================================*/
void read_points(PointList& points){
    string line;
    string point_data = "";
    while(getline(cin, line)){
        if(line[0] != '#')
            point_data += line + ' ';
    }
    
    istringstream iss(point_data);
    int x;
    int y;
    int index = 0;
    while(iss >> x >> y){
        points.push_back(new PointNode(x, y, index++));
    }
}

inline long distance_squared(const PointNode& p1, const PointNode& p2){
//  Faster than floats, as no values are in interval (0, 1)
    long dx = p1.x - p2.x;
    long dy = p1.y - p2.y;
    return dx*dx + dy*dy;
}

float path_distance(const PointList& path){
    float distance_sum = 0.0;
    for(int i = 0; i < path.size() - 1; ++i)
        distance_sum += path[i]->distance_to(*path[i+1]);
    return distance_sum;
}

PointList find_shortest_path(PointList& points){
    PointList path;
    float shortest_distance = numeric_limits<float>::max();
    if(points.size() < SIZE_TO_SEARCH){
    //  For small set, brute force check for best segment division
        PointList current_path;
        float current_distance;
        for(int i = 2; i < 22; i += 2){
            current_path = segment_method_path(points, i);
            current_distance = path_distance(current_path);
            if(shortest_distance < current_distance)
                break;
            path = current_path;
            shortest_distance = current_distance;
        }
    }
    else{
    //  For large set, make a guess; based on statistical analysis
        int num_of_segments = 14 + 2 * nearbyint(1/200.0 *
                                         (points.size() - SIZE_TO_SEARCH));
        path = segment_method_path(points, num_of_segments);
    }
    return path;
}

void shrink_scope(PointList& points){
//  Reduces coordinate window to later obtain a good number of segments
    int lowX = numeric_limits<int>::max();
    int lowY = numeric_limits<int>::max();
    for(int i = 0; i < points.size(); ++i){
        if(points[i]->x < lowX)
            lowX = points[i]->x;
        if(points[i]->y < lowY)
            lowY = points[i]->y;
    }
    
    for(int i = 0; i < points.size(); ++i){
        points[i]->x -= lowX;
        points[i]->y -= lowY;
    }
}

PointList segment_method_path(PointList& points, int num_of_segments){
/*  My own personal heuristic
 *  Divides Quadrant 1 into even segments radially
 *  Determine which segment each point goes into
 *  Sort even-numbered segments in ascending order, odd in descending
 *  The merged segments is a path.
 *  Runs in O(n log n) and is reasonable for good number of segments
 */
    vector<PointList> segments;
    segments.reserve(num_of_segments);
    for(int i = 0; i < num_of_segments; ++i)
        segments.push_back(PointList());
    
    fill_segments(points, segments);
    return path_from_segments(segments);
}

void fill_segments(PointList& points, vector<PointList>& segments){
//  Determine to which segment each point belongs
    PointNode origin(0, 0, -1);
    for(int i = 0; i < points.size(); ++i){
        PointNodePtr p = points[i];
        p->key = distance_squared(origin, *p);
        int segment_num = 0;
        if(p->x == 0)  // avoid divide by 0
            segment_num = segments.size() - 1;
        else
            segment_num = floor(atan(p->y / (float)(p->x)) * 
                                2 * segments.size() / M_PI);
        segments[segment_num].push_back(p);
    }
}

PointList path_from_segments(vector<PointList>& segments){
//  Sort the segments into a path.  The resultant path is like a giant zigzag
    PointList path;
    path.reserve(MAX_POINTS);
    
    for(int i = 0; i < segments.size(); ++i){
        if(i % 2 == 0)
            sort(segments[i].begin(), segments[i].end(), PointNodeLess());
        else
            sort(segments[i].begin(), segments[i].end(), PointNodeGreater());
            
        path.insert(path.end(), segments[i].begin(), segments[i].end());
    }
    
    path.push_back(path.front());  // make into a loop
    
    return path;
}

inline void print_path(const PointList& path){
    for(int i = 0; i < path.size(); ++i)
        cout << path[i]->index << endl;
}
