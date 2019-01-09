#include <Path.hpp>

#include <iostream>

#include <cmath> // FIXME remove me!

void Path::updateLength() {
    if (nodes_.empty()) {
        length_ = 0;
        return;
    }

    length_ = nodes_[0]->length;
    for (uint i = 1; i < edges_.size(); i++) {
        length_ += nodes_[i]->length - (long) edges_[i - 1]->q_end
                   - (nodes_[i - 1]->length - (long) edges_[i - 1]->t_end);
    }
    length_ = std::abs(length_); // FIXME REMOVE ME!
}

long Path::length() const {
    return length_;
}

std::ostream &operator<<(std::ostream &s, const Path &p) {
    bool e = false;
    int ni = 0;
    for (int i = 0, n = static_cast<int>(p.nodes_.size() + p.edges_.size());
         i < n;
         i++) {
        if (e) {
            s << "-";
            e = false;
        } else {
            if (p.nodes_[ni]->anchor) {
                s << '*';
            }
            s << 'n' << p.nodes_[ni]->index;
            ni++;
            e = true;
        }
    }
    return s;
}

bool Path::operator==(const Path &other) const {
    return nodes_.size() == other.nodes_.size()
           && std::equal(nodes_.begin(), nodes_.end(), other.nodes_.begin());
}
