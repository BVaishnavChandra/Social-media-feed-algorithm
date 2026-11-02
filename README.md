# Personalized Social Media Feed Algorithm

A C++ console project that ranks social media posts and changes future recommendations based on the user's likes. It demonstrates vectors, linked lists, stacks, queues, pairs, and hash tables through a small feed simulation.

## Features

- Rank unseen public posts using engagement, freshness, topic preferences, and a following bonus.
- Select posts in batches of four and display them one at a time.
- Prefer at most two posts from each author per batch, relaxing this limit when necessary to fill the batch.
- Increase a topic's preference when the user likes a post.
- Prevent duplicate likes and repeated posts during the same run.
- Undo the latest like, reversing both the like count and its topic-preference increase.
- Display viewing history and learned topic preferences.
- Handle invalid menu input without skipping posts.

## Data structures used

| Data structure | C++ implementation | Use in this project |
| --- | --- | --- |
| Arrays / Vectors | `vector<Post>`, `vector<RankedPost>` | Store sample posts and ranking candidates |
| Linked list | `list<int>` | Store viewed post IDs in viewing order |
| Stack | `stack<pair<int, string>>` | Record successful likes so the latest can be undone first (LIFO) |
| Queue | `queue<RankedPost>` | Deliver selected posts in their ranked order (FIFO) |
| Maps / Pairs | `unordered_map`, `pair<int, string>` | Associate topics with preferences; store a liked post's ID and topic together |
| Hash tables | `unordered_map`, `unordered_set` | Track preferences, author counts, followed accounts, shown posts, and liked posts |

The containers come from the C++ Standard Library. Ranking uses `sort()`. The project does not implement a binary tree, BST, or heap, and does not use dynamic programming.

## How the feed works

1. Start with one viewer and 12 sample posts. The viewer initially follows nobody and has no known interests.
2. Filter out private posts and posts already displayed to the viewer.
3. Calculate a score for each remaining post and sort by highest score. Prefer newer posts when scores tie.
4. Apply the author-variety rule and put up to four selected posts into a queue.
5. Display the front post, remove it from the queue, and append its ID to viewing history.
6. Let the viewer like the current post, undo a like, inspect history, or move forward.
7. When the queue is empty and the viewer requests another post, rank the remaining unseen posts using the updated preferences.

Likes and undo affect the next batch. Posts already queued retain their order and the score calculated when that batch was built.

## Scoring formula

```text
Engagement = likes + 2 * comments + 3 * shares
Freshness = 100 / (1 + max(0, minutesAgo) / 60.0)
Interest bonus = 30 * topic preference
Following bonus = 20 if the viewer follows the author, otherwise 0

Score = Engagement + Freshness + Interest bonus + Following bonus
```

Each successful like increases that topic's preference by 1 and the post's like count by 1. For example, liking a technology post adds 30 points to the interest bonus of unseen technology posts when the next batch is ranked. Undo reverses the corresponding changes.

These weights are demonstration rules. This is a rule-based algorithm, not a trained machine-learning model or a reproduction of Instagram's algorithm.

## Compile and run

Use a C++17-compatible compiler. Open a terminal in the folder containing `personalized_social_feed.cpp`.

### Windows PowerShell

```powershell
g++ -std=c++17 personalized_social_feed.cpp -o feed.exe
.\feed.exe
```

### Linux or macOS

```bash
g++ -std=c++17 personalized_social_feed.cpp -o feed
./feed
```

## Menu

The first post appears automatically. Enter a menu number, not a post ID.

| Choice | Action |
| --- | --- |
| `1` | Show the next post |
| `2` | Like the current post |
| `3` | Undo the latest successful like |
| `4` | Show viewing history, oldest first |
| `5` | Show learned topic preferences |
| `0` | Exit |

Liking does not advance the feed. You can like different posts in the same batch, but cannot like the same post again unless its earlier like has been undone.

## Quick demonstration

1. Run the program. The first post is technology post `107`.
2. Enter `2` to like it, then `5` to see the technology preference become `1`.
3. Enter `1` four times to reach the first post of the next batch. With that like, it is technology post `105`; without the like, it would be sports post `108`.
4. Enter `4` to inspect viewing history, or `3` to undo your latest like.

## Scope and limitations

- All data is stored in memory and resets when the program exits.
- The sample contains 11 public posts and one private post. It does not fetch new posts from a server.
- Post ages, comments, and shares are preset values; they do not change with elapsed time or menu actions.
- The following bonus is implemented, but the demo starts with no follows and has no follow/unfollow menu.
- Interests start empty and are learned from likes; there is no signup or initial interest-selection screen.
- Author variety is a soft per-batch limit, not a guarantee that adjacent posts have different authors.
- After all public posts are viewed, history, preferences, and undo remain available until exit.

