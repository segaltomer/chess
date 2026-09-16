#include "Matchmaker.h"

#include <algorithm>


bool Matchmaker::isWaiting(int clientId) const {
    return std::find(waitingClients_.begin(), waitingClients_.end(), clientId)
           != waitingClients_.end();
}


bool Matchmaker::addWaitingClient(int clientId) {
    if (isWaiting(clientId)) {
        return false;
    }

    waitingClients_.push_back(clientId);
    return true;
}


bool Matchmaker::removeWaitingClient(int clientId) {
    const auto waiting = std::find(waitingClients_.begin(), waitingClients_.end(),
                                   clientId);
    if (waiting == waitingClients_.end()) {
        return false;
    }

    waitingClients_.erase(waiting);
    return true;
}


bool Matchmaker::tryPair(int& firstClientId, int& secondClientId) {
    if (waitingClients_.size() < 2) {
        return false;
    }

    firstClientId = waitingClients_[0];
    secondClientId = waitingClients_[1];

    waitingClients_.erase(waitingClients_.begin(), waitingClients_.begin() + 2);

    return true;
}


std::size_t Matchmaker::waitingCount() const {
    return waitingClients_.size();
}
