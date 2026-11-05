# Personalized Social Media Feed Algorithm

A C++ console project for a Data Structures and Algorithms lab. It ranks sample social media posts using engagement, recency, and each viewer's interests. Liking a post increases the preference for its topic, influencing future feed batches.

## Features

- Create viewers with a unique ID, name, and selected interests.
- Switch between viewers while preserving their progress during the current run.
- Rank posts in batches of three and display them one at a time.
- Exclude private posts and posts already viewed by the active viewer.
- Like the current post, prevent duplicate likes, and undo the most recent like.
- View browsing history and current topic preference scores.
- Prefer at most two posts from one author per batch, allowing more when no other author is available.

The demo contains 20 sample posts across six topics: technology, sports, food, wellness, travel, and education. Two posts are private and are excluded from the feed.

## Ranking Formula

```text
Score = Engagement + Recency + Topic Bonus + Follow Bonus

Engagement = Likes + 2 × Comments + 3 × Shares
Recency    = 100 / (1 + max(0, Minutes Ago) / 60)
```

| Component | Value or behavior |
| --- | --- |
| Selected interest | Starts with a topic bonus of 25 points |
| Each like by the viewer | Adds 5 points to that viewer's preference for the post's topic |
| Post's like count | Increases by 1 when liked, adding 1 engagement point |
| Followed author | Adds 25 points if the author is in the viewer's following set |
| Equal scores | The newer post is ranked first |

An unselected topic starts with no bonus but can gain points through likes. Undoing a like reverses both the post's extra like and the viewer's 5-point topic increase.

**Batch behavior:** Likes and undo actions affect newly generated batches. Posts already waiting in the queue retain their existing order and stored scores.

**Follow bonus:** The formula includes it, but this version has no follow/unfollow menu and starts with empty following sets. Therefore, the follow bonus is zero in the default demo.

## Data Structures Used

| Data structure | Where it is used | Purpose |
| --- | --- | --- |
| Vector (`vector`) | Sample posts, topics, and ranking candidates | Stores collections and allows candidates to be sorted by score |
| Doubly linked list (`list<int>`) | Each viewer's viewing history | Appends viewed post IDs and preserves viewing order |
| Stack (`stack<pair<int, string>>`) | Each viewer's like history | Supports undoing likes in last-in, first-out order |
| Queue (`queue<RankedPost>`) | Each viewer's pending feed | Displays the ranked batch in first-in, first-out order |
| Hash map (`unordered_map`) | Viewer profiles, topic bonuses, and author counts | Looks up profiles and scores and tracks author frequency |
| Hash set (`unordered_set`) | Viewed posts, liked posts, and followed authors | Checks membership and prevents repeat views or duplicate likes |
| Pair (`pair<int, string>`) | An entry in the like stack | Keeps the post ID and topic together for undo |

This version ranks candidates with `std::sort`; it does not use a heap or binary search tree.

## Build and Run

Requires a C++ compiler with C++14 support or later, such as GCC with `g++`.

Open a terminal in the folder containing `personalized_social_feed_algorithm.cpp`.

### Windows (PowerShell)

```powershell
g++ -std=c++14 personalized_social_feed_algorithm.cpp -o feed.exe
.\feed.exe
```

### Linux / macOS

```bash
g++ -std=c++14 personalized_social_feed_algorithm.cpp -o feed
./feed
```

## Using the Program

1. Enter a viewer ID, such as `ID201`.
2. For a new viewer, enter a name and choose topic numbers. Enter `0` when finished selecting interests.
3. The first post appears automatically. Use the menu below to interact.

| Option | Action |
| --- | --- |
| 1 | Next post |
| 2 | Like current post |
| 3 | Undo last like |
| 4 | Show viewing history |
| 5 | Show topic preferences |
| 6 | Switch viewer or create another viewer |
| 0 | Exit |

## Scope

This is an educational simulation using predefined posts and rule-based scoring. All viewer data is kept in memory and resets when the program exits. Post ages are fixed sample values; they do not advance with real time. The program has no database, authentication, live social media connection, or post-creation interface.
