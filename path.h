#ifndef PATH_H
#define PATH_H

#include <vector>
#include <stdexcept>
#include <initializer_list>
#include <utility>   // for std::move
#include <QString>

/// \brief Represents a single 2D point with real coordinates
struct Point {
    double x;
    double y;

    Point(double xx=0.0, double yy=0.0) : x(xx), y(yy) {}
};

/// \brief Represents a path as a sequence of 2D points.
class Path {
public:
    // Constructors
    Path() = default;
    Path(std::initializer_list<Point> pts);

    // Rule of Five
    Path(const Path& other);
    Path(Path&& other) noexcept;
    Path& operator=(const Path& other);
    Path& operator=(Path&& other) noexcept;
    ~Path() = default;

    // Add point
    void addPoint(double x, double y);

    // Accessors
    const Point& at(size_t idx) const;
    size_t size() const;
    void clear();

    // Iterators
    const std::vector<Point>& points() const { return points_; }
    std::vector<Point>& points() { return points_; }

    void saveAsGeoJSON(const QString& filename, int crsEPSG = 4326) const;
    void loadFromGeoJSON(const QString& filename);
private:
    std::vector<Point> points_;
};

#endif // PATH_H
