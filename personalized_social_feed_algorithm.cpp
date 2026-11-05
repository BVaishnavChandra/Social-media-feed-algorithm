#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iomanip>
#include <list>
#include <stack>
#include <queue>
#include <utility>
#include <sstream>

using namespace std;

constexpr int SELECTED_TOPIC_BONUS = 25;
constexpr int TOPIC_BONUS_PER_LIKE = 5;

struct Post {
    int id;
    int authorId;
    string text;
    string topic;
    int minutesAgo;
    int likes;
    int comments;
    int shares;
    bool isPublic = true;
};

struct RankedPost {
    Post post;
    double score;
};

struct User {
    string id;
    string name;
    unordered_set<int> following;
    unordered_map<string, int> interests;
    unordered_set<int> shownPostIds;
    unordered_set<int> likedPostIds;
    list<int> viewingHistory;
    stack<pair<int, string>> likeHistory;
    queue<RankedPost> pendingFeed;
    RankedPost current{};
    bool hasCurrent = false;
};

class FeedAlgorithm {
private:
    static constexpr double ENGAGEMENT_WEIGHT = 1.0;
    static constexpr double RECENCY_WEIGHT = 100.0;
    static constexpr double FOLLOW_WEIGHT = 25.0;

    double calculateScore(const Post& post, const User& viewer) const {

        double engagement = post.likes + 2.0 * post.comments + 3.0 * post.shares;

        double recency = RECENCY_WEIGHT / (1.0 + max(0, post.minutesAgo) / 60.0);

        double interestScore = 0.0;
        auto interest = viewer.interests.find(post.topic);
        if (interest != viewer.interests.end()) {
            interestScore = interest->second;
        }

        double followScore =
            viewer.following.count(post.authorId) ? FOLLOW_WEIGHT : 0.0;

        return ENGAGEMENT_WEIGHT * engagement
             + recency
             + interestScore
             + followScore;
    }

public:
    queue<RankedPost> getNextBatch(
        User& viewer,
        const vector<Post>& allPosts,
        int batchSize
    ) const {
        vector<RankedPost> candidates;

        for (const Post& post : allPosts) {
            if (!post.isPublic) continue;
            if (viewer.shownPostIds.count(post.id)) continue;

            candidates.push_back({post, calculateScore(post, viewer)});
        }

        sort(candidates.begin(), candidates.end(),
             [](const RankedPost& a, const RankedPost& b) {
                 if (a.score != b.score) return a.score > b.score;
                 return a.post.minutesAgo < b.post.minutesAgo;
             });

        queue<RankedPost> batch;
        unordered_map<int, int> authorCount;

        while (!candidates.empty() &&
               static_cast<int>(batch.size()) < batchSize) {
            int chosen = -1;

            for (int i = 0; i < static_cast<int>(candidates.size()); ++i) {
                if (authorCount[candidates[i].post.authorId] < 2) {
                    chosen = i;
                    break;
                }
            }

            if (chosen == -1) chosen = 0;

            RankedPost selected = candidates[chosen];
            batch.push(selected);
            authorCount[selected.post.authorId]++;

            candidates.erase(candidates.begin() + chosen);
        }

        return batch;
    }

    bool recordPositiveInteraction(User& viewer, Post& post) const {
        if (viewer.likedPostIds.count(post.id)) return false;
        viewer.likedPostIds.insert(post.id);
        viewer.interests[post.topic] += TOPIC_BONUS_PER_LIKE;
        post.likes++;
        viewer.likeHistory.push(make_pair(post.id, post.topic));
        return true;
    }

    bool undoLike(User& viewer, vector<Post>& posts) const {
        if (viewer.likeHistory.empty()) return false;
        pair<int, string> action = viewer.likeHistory.top();
        for (Post& post : posts) {
            if (post.id == action.first) {
                post.likes--;
                viewer.likedPostIds.erase(post.id);
                viewer.interests[action.second] -= TOPIC_BONUS_PER_LIKE;
                if (viewer.interests[action.second] == 0)
                    viewer.interests.erase(action.second);
                viewer.likeHistory.pop();
                return true;
            }
        }
        return false;
    }
};

int readChoice(const string& prompt = "Choose: ") {
    string line, extra;
    int choice;
    while (true) {
        cout << prompt;
        if (!getline(cin, line)) return 0;
        stringstream input(line);
        if ((input >> choice) && !(input >> extra)) return choice;
        cout << "Enter a whole number.\n";
    }
}

void printPost(const RankedPost& item) {
    cout << "\nPost ID: " << item.post.id
         << " | Author: " << item.post.authorId
         << " | Topic: " << item.post.topic
         << " | Score when batch was built: " << fixed << setprecision(1)
         << item.score << "\n" << item.post.text << "\n";
}

void showHistory(const User& viewer, const vector<Post>& posts) {
    if (viewer.viewingHistory.empty()) {
        cout << "No posts viewed yet.\n";
        return;
    }
    cout << "\nViewing history, oldest first:\n";
    for (int id : viewer.viewingHistory) {
        for (const Post& post : posts) {
            if (post.id == id) {
                cout << post.id << " | " << post.topic << " | " << post.text << '\n';
                break;
            }
        }
    }
}

void showInterests(const User& viewer) {
    cout << "\nTopic bonuses (ranking points):\n";
    if (viewer.interests.empty()) cout << "None yet.\n";
    for (const auto& entry : viewer.interests)
        cout << entry.first << ": " << entry.second << '\n';
}

string selectViewer(unordered_map<string, User>& profiles, const vector<string>& topics) {
    string line, id, extra;
    while (true) {
        cout << "Viewer ID (for example ID201, or 0 to exit): ";
        if (!getline(cin, line)) return "";
        stringstream input(line);
        if ((input >> id) && !(input >> extra)) break;
        cout << "Enter one ID without spaces.\n";
    }
    if (id == "0") return "";

    if (profiles.count(id) == 0) {
        User profile;
        profile.id = id;
        do {
            cout << "New viewer. Enter your name: ";
            if (!getline(cin, profile.name)) return "";
        } while (profile.name.find_first_not_of(" \t\r") == string::npos);

        cout << "Choose your interests one at a time. Enter 0 when done.\n";
        for (size_t i = 0; i < topics.size(); ++i)
            cout << i + 1 << ". " << topics[i] << '\n';
        while (true) {
            int choice = readChoice("Topic number (0 to finish): ");
            if (!cin) return "";
            if (choice == 0) break;
            if (choice < 1 || static_cast<size_t>(choice) > topics.size()) {
                cout << "Choose a listed topic number.\n";
                continue;
            }
            const string& topic = topics[choice - 1];
            profile.interests[topic] = SELECTED_TOPIC_BONUS;
            cout << "Selected " << topic << ".\n";
        }
        profiles.emplace(id, std::move(profile));
    }
    cout << "\nViewer: " << profiles.at(id).name << " (" << id << ")\n";
    return id;
}

int main() {
    vector<Post> posts = {
        {101, 2, "A quick introduction to C++ vectors.", "technology", 10, 30, 5, 2},
        {102, 8, "Highlights from today's football match.", "sports", 25, 80, 12, 9},
        {103, 4, "A simple recipe for a quick lunch.", "food", 15, 25, 4, 3},
        {104, 9, "Why sleep matters for learning.", "wellness", 5, 40, 10, 4},
        {105, 3, "A beginner-friendly machine learning project.", "technology", 50, 55, 8, 6},
        {106, 7, "A beautiful mountain trail.", "travel", 8, 20, 3, 1},
        {107, 2, "Three useful debugging habits.", "technology", 90, 100, 20, 12},
        {108, 5, "A new basketball training drill.", "sports", 12, 45, 7, 5},
        {109, 6, "A post from a private account.", "technology", 1, 500, 100, 50, false},
        {110, 10, "How to make better study notes.", "education", 3, 35, 6, 2},
        {111, 8, "A second football analysis.", "sports", 18, 60, 9, 7},
        {112, 11, "A beginner's guide to photography.", "education", 22, 28, 4, 2},
        {113, 4, "Three ways to use leftover rice.", "food", 20, 32, 5, 3},
        {114, 7, "A weekend walking route around Hyderabad.", "travel", 30, 35, 6, 4},
        {115, 2, "How I tracked down a bug with a breakpoint.", "technology", 35, 28, 6, 3},
        {116, 9, "A five-minute stretch between classes.", "wellness", 18, 65, 11, 8},
        {117, 10, "How to quiz yourself before a viva.", "education", 20, 71, 13, 7},
        {118, 4, "Making dosa batter in a hostel kitchen.", "food", 16, 65, 11, 9},
        {119, 7, "A day trip by train from Hyderabad.", "travel", 20, 68, 12, 8},
        {120, 5, "Our team's private practice schedule.", "sports", 2, 200, 25, 15, false}
    };

    FeedAlgorithm algorithm;
    const int batchSize = 3;
    const vector<string> topics = {"technology", "sports", "food", "wellness", "travel", "education"};
    unordered_map<string, User> profiles;

    cout << "Personalized Social Media Feed Demo\n";
    cout << "Posts are ranked in batches of three, displayed one at a time.\n";

    string activeId = selectViewer(profiles, topics);
    bool moveNext = true;
    while (!activeId.empty()) {
        User& viewer = profiles.at(activeId);
        auto& pendingFeed = viewer.pendingFeed;
        auto& current = viewer.current;
        bool& hasCurrent = viewer.hasCurrent;
        if (moveNext) {
            if (pendingFeed.empty()) {
                pendingFeed = algorithm.getNextBatch(viewer, posts, batchSize);
                if (!pendingFeed.empty()) cout << "\nNew batch ranked.\n";
            }
            hasCurrent = !pendingFeed.empty();
            if (hasCurrent) {
                current = pendingFeed.front();
                pendingFeed.pop();
                viewer.shownPostIds.insert(current.post.id);
                viewer.viewingHistory.push_back(current.post.id);
                printPost(current);
            } else {
                cout << "\nNo more unseen public posts. History and undo are still available.\n";
            }
            moveNext = false;
        }

        cout << "\n1. Next post   2. Like current post   3. Undo last like\n"
             << "4. Viewing history   5. Topic preferences   6. Switch viewer   0. Exit\n";
        int choice = readChoice();
        if (choice == 0) break;

        switch (choice) {
        case 1:
            moveNext = true;
            break;
        case 2:
            if (!hasCurrent) {
                cout << "No current post to like.\n";
                break;
            }
            for (Post& post : posts) {
                if (post.id == current.post.id) {
                    if (algorithm.recordPositiveInteraction(viewer, post))
                        cout << "Liked. Preference increased for " << post.topic << ".\n";
                    else
                        cout << "You already liked this post.\n";
                    break;
                }
            }
            break;
        case 3:
            if (algorithm.undoLike(viewer, posts))
                cout << "Latest like and its topic preference increase were undone.\n";
            else
                cout << "No likes to undo.\n";
            break;
        case 4:
            showHistory(viewer, posts);
            break;
        case 5:
            showInterests(viewer);
            break;
        case 6:
            activeId = selectViewer(profiles, topics);
            if (!activeId.empty()) {
                const User& selected = profiles.at(activeId);
                moveNext = !selected.hasCurrent;
                if (selected.hasCurrent) printPost(selected.current);
            }
            break;
        default:
            cout << "Choose a number from 0 to 6.\n";
        }
    }

    cout << "\nDemo ended.\n";
    return 0;
}
