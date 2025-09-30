#include "junction.h"

Junction::Junction() : location_(0.0, 0.0) {}

Junction::Junction(const QPointF& location) : location_(location) {}

Junction::Junction(double x, double y) : location_(x, y) {}

Junction::Junction(const QPointF& location, const QMap<QString, QVariant>& attributes)
    : location_(location), attributes_(attributes) {}

Junction::Junction(const Junction& other)
    : location_(other.location_),
    connectedPolylines_(other.connectedPolylines_),
    attributes_(other.attributes_) {}

Junction& Junction::operator=(const Junction& other) {
    if (this != &other) {
        location_ = other.location_;
        connectedPolylines_ = other.connectedPolylines_;
        attributes_ = other.attributes_;
    }
    return *this;
}

Junction::Junction(Junction&& other) noexcept
    : location_(std::move(other.location_)),
    connectedPolylines_(std::move(other.connectedPolylines_)),
    attributes_(std::move(other.attributes_)) {}

Junction& Junction::operator=(Junction&& other) noexcept {
    if (this != &other) {
        location_ = std::move(other.location_);
        connectedPolylines_ = std::move(other.connectedPolylines_);
        attributes_ = std::move(other.attributes_);
    }
    return *this;
}

// Location accessors
const QPointF& Junction::getLocation() const {
    return location_;
}

void Junction::setLocation(const QPointF& location) {
    location_ = location;
}

void Junction::setLocation(double x, double y) {
    location_ = QPointF(x, y);
}

double Junction::x() const {
    return location_.x();
}

double Junction::y() const {
    return location_.y();
}

// Polyline connection management
void Junction::addConnectedPolyline(std::shared_ptr<Polyline> polyline) {
    if (polyline && !isConnectedTo(polyline)) {
        connectedPolylines_.append(polyline);
    }
}

void Junction::removeConnectedPolyline(std::shared_ptr<Polyline> polyline) {
    connectedPolylines_.removeAll(polyline);
}

bool Junction::isConnectedTo(std::shared_ptr<Polyline> polyline) const {
    return connectedPolylines_.contains(polyline);
}

const QVector<std::shared_ptr<Polyline>>& Junction::getConnectedPolylines() const {
    return connectedPolylines_;
}

int Junction::getConnectionCount() const {
    return connectedPolylines_.size();
}

bool Junction::hasConnections() const {
    return !connectedPolylines_.isEmpty();
}

// Attribute management
void Junction::setAttribute(const QString& name, const QVariant& value) {
    attributes_[name] = value;
}

QVariant Junction::getAttribute(const QString& name) const {
    return attributes_.value(name);
}

QVariant Junction::getAttribute(const QString& name, const QVariant& defaultValue) const {
    return attributes_.value(name, defaultValue);
}

bool Junction::hasAttribute(const QString& name) const {
    return attributes_.contains(name);
}

void Junction::removeAttribute(const QString& name) {
    attributes_.remove(name);
}

void Junction::clearAttributes() {
    attributes_.clear();
}

const QMap<QString, QVariant>& Junction::getAllAttributes() const {
    return attributes_;
}

QStringList Junction::getAttributeNames() const {
    return attributes_.keys();
}

// Convenience methods for common attribute types
void Junction::setNumericAttribute(const QString& name, double value) {
    attributes_[name] = value;
}

void Junction::setStringAttribute(const QString& name, const QString& value) {
    attributes_[name] = value;
}

void Junction::setIntAttribute(const QString& name, int value) {
    attributes_[name] = value;
}

void Junction::setBoolAttribute(const QString& name, bool value) {
    attributes_[name] = value;
}

double Junction::getNumericAttribute(const QString& name, double defaultValue) const {
    return attributes_.value(name, defaultValue).toDouble();
}

QString Junction::getStringAttribute(const QString& name, const QString& defaultValue) const {
    return attributes_.value(name, defaultValue).toString();
}

int Junction::getIntAttribute(const QString& name, int defaultValue) const {
    return attributes_.value(name, defaultValue).toInt();
}

bool Junction::getBoolAttribute(const QString& name, bool defaultValue) const {
    return attributes_.value(name, defaultValue).toBool();
}

// Distance calculations
double Junction::distanceTo(const QPointF& point) const {
    double dx = location_.x() - point.x();
    double dy = location_.y() - point.y();
    return std::sqrt(dx * dx + dy * dy);
}

double Junction::distanceTo(const Junction& other) const {
    return distanceTo(other.location_);
}

bool Junction::isWithinDistance(const QPointF& point, double tolerance) const {
    return distanceTo(point) <= tolerance;
}

bool Junction::isWithinDistance(const Junction& other, double tolerance) const {
    return distanceTo(other) <= tolerance;
}

// Comparison operators
bool Junction::operator==(const Junction& other) const {
    return location_ == other.location_;
}

bool Junction::operator!=(const Junction& other) const {
    return !(*this == other);
}

// Utility methods
bool Junction::isEmpty() const {
    return connectedPolylines_.isEmpty();
}

void Junction::clear() {
    connectedPolylines_.clear();
    attributes_.clear();
}

QString Junction::toString() const {
    return QString("Junction at (%1, %2) with %3 connections and %4 attributes")
    .arg(location_.x())
        .arg(location_.y())
        .arg(connectedPolylines_.size())
        .arg(attributes_.size());
}
