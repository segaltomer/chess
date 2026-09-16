#pragma once

#include <cstddef>
#include <vector>

// The queue of Clients waiting to be paired into an online game (§40-42).
//
// §42 defines a "free" Client as one that is connected, has asked for an online
// game, and is not already in one. The first and last of those are the Server's
// business — it owns the connections and the games — so this class is
// responsible for exactly the middle one: who has asked and is still asking.
// The Server does not put a Client in here unless the other two hold.
//
// A vector rather than a queue, because the waiting list is not only added to
// and taken from in order: a Client can also leave from the middle of it, by
// cancelling or by disconnecting (§42), and that is the case a queue handles
// worst.
class Matchmaker {
public:
    // §40: start waiting. False if this Client was already in the list, which
    // means a repeated request rather than a new one (§67).
    bool addWaitingClient(int clientId);

    // §42: stop waiting, whether by cancelling or by disconnecting. False if
    // the Client was not waiting in the first place.
    bool removeWaitingClient(int clientId);

    bool isWaiting(int clientId) const;

    // §41: if two Clients are waiting, removes both and reports them. They come
    // out in the order they arrived, so nobody is passed over indefinitely.
    // False, and nothing removed, when fewer than two are waiting.
    bool tryPair(int& firstClientId, int& secondClientId);

    std::size_t waitingCount() const;

private:
    std::vector<int> waitingClients_;
};
