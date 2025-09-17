#include "streamnetwork.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

// --- Constructor ---
StreamNetwork::StreamNetwork(const std::vector<Node>& nodes)
    : nodes_(nodes) {}

// --- Build directed network ---
StreamNetwork StreamNetwork::buildDirected(const std::vector<Node>& nodes) {
    StreamNetwork net(nodes);

    int n = static_cast<int>(nodes.size());

    // indices sorted by descending value
    std::vector<int> idx(n);
    for (int i = 0; i < n; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(),
              [&](int a, int b){ return nodes[a].value > nodes[b].value; });

    // For each node (high -> low), find nearest lower node
    for (int k = 0; k < n; ++k) {
        int i = idx[k];
        double bestDist2 = std::numeric_limits<double>::infinity();
        int bestJ = -1;

        for (int m = k+1; m < n; ++m) { // only lower-valued nodes
            int j = idx[m];
            double dx = nodes[i].x - nodes[j].x;
            double dy = nodes[i].y - nodes[j].y;
            double d2 = dx*dx + dy*dy;
            if (d2 < bestDist2) {
                bestDist2 = d2;
                bestJ = j;
            }
        }

        if (bestJ != -1) {
            net.addEdge(i, bestJ);
        }
    }

    return net;
}

// --- Add edge ---
void StreamNetwork::addEdge(int from, int to) {
    if (from < 0 || from >= static_cast<int>(nodes_.size()) ||
        to   < 0 || to   >= static_cast<int>(nodes_.size())) {
        throw std::out_of_range("Invalid node index for edge.");
    }
    edges_.emplace_back(from, to);
}

// --- Getters ---
const std::vector<Node>& StreamNetwork::nodes() const { return nodes_; }
const std::vector<StreamNetwork::Edge>& StreamNetwork::edges() const { return edges_; }
int StreamNetwork::nodeCount() const { return static_cast<int>(nodes_.size()); }
int StreamNetwork::edgeCount() const { return static_cast<int>(edges_.size()); }

// --- Print summary ---
void StreamNetwork::printSummary(std::ostream& os) const {
    os << "Stream Network Summary:\n";
    os << "  Nodes: " << nodeCount() << "\n";
    os << "  Edges: " << edgeCount() << "\n";

    for (int i = 0; i < nodeCount(); ++i) {
        os << "    " << i << ": " << nodes_[i] << "\n";
    }

    for (auto& e : edges_) {
        os << "    Edge: " << e.first << " -> " << e.second << "\n";
    }
}
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

void StreamNetwork::saveEdgesAsGeoJSON(const QString& fileName) const {
    QJsonArray features;

    // --- Add edge features only ---
    for (auto& e : edges_) {
        int from = e.first;
        int to   = e.second;
        if (from < 0 || from >= nodeCount() || to < 0 || to >= nodeCount()) continue;

        const Node& a = nodes_[from];
        const Node& b = nodes_[to];

        QJsonObject geometry;
        geometry["type"] = "LineString";

        QJsonArray coords;
        QJsonArray pt1; pt1.append(a.x); pt1.append(a.y);
        QJsonArray pt2; pt2.append(b.x); pt2.append(b.y);

        coords.append(pt1);
        coords.append(pt2);
        geometry["coordinates"] = coords;

        QJsonObject properties;
        properties["from"] = from;
        properties["to"]   = to;
        properties["from_value"] = a.value;
        properties["to_value"]   = b.value;

        QJsonObject feature;
        feature["type"] = "Feature";
        feature["geometry"] = geometry;
        feature["properties"] = properties;

        features.append(feature);
    }

    // --- FeatureCollection ---
    QJsonObject fc;
    fc["type"] = "FeatureCollection";
    fc["features"] = features;

    QJsonDocument doc(fc);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Failed to open file for writing: " + fileName.toStdString());
    }
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}
