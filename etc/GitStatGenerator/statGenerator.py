from git import Repo
import os

users = dict()
total_insertion = list()
net_contributions = list()

OverallTimeline = dict()
UserTimeline = dict()

projectRoot = os.path.dirname(__file__) + "/../../"
print("Script running at", projectRoot)
repo = Repo(projectRoot)

print("Reading commits...")

fout = open(projectRoot + "build/contriStat.csv", "w", -1, "UTF-8")
counter = 0

# Get commmits and count contributions in lines
for commit in repo.iter_commits():
    counter += 1
    commitDate = str(commit.committed_datetime).split(" ")[0]
    print("Processing commit", commit.hexsha[0:8], "at", commitDate, end="\r")
    if commit.author.name not in users:
        users[commit.author.name] = len(total_insertion)
        total_insertion.append(0)
        net_contributions.append(0)
    total_insertion[users[commit.author.name]] += commit.stats.total["insertions"]
    net_contributions[users[commit.author.name]] += (
            commit.stats.total["insertions"] - commit.stats.total["deletions"]
    )

    if not (commitDate in OverallTimeline):
        OverallTimeline[commitDate] = list([0, 0, 0])
    OverallTimeline[commitDate][0] += 1
    OverallTimeline[commitDate][1] += commit.stats.total["insertions"]
    OverallTimeline[commitDate][2] += commit.stats.total["deletions"]
del commit

for user in users:
    print("                                                        ", end="\r")
    print("Processing user", user)
    UserTimeline[user] = dict()
    for commit in repo.iter_commits():
        commitDate = str(commit.committed_datetime).split(" ")[0]
        print("Processing commit", commit.hexsha[0:8], "at", commitDate, end="\r")

        if not (commitDate in UserTimeline[user]):
            UserTimeline[user][commitDate] = 0

        if commit.author.name == user:
            UserTimeline[user][commitDate] += commit.stats.total["insertions"]

print("Phrased", counter, "commits. Writing...")

fout.write("Contribution in lines\n")
fout.write("Username, Total insertion, Net contribution\n")

for user in users:
    fout.write(
        user
        + ", "
        + str(total_insertion[users[user]])
        + ", "
        + str(net_contributions[users[user]])
        + "\n"
    )
fout.write("\n")

fout.write("Heat Timeline\n")
fout.write("Date, Commits count\n")
for day in OverallTimeline:
    fout.write(
        str(day)
        + ", "
        + str(OverallTimeline[day][0])
        + "\n")
del day

fout.write("\n\nInsertion/Deletion Timeline\n")
fout.write("Date, Insertion, Deletion\n")
for day in OverallTimeline:
    fout.write(
        str(day)
        + ", "
        + str(OverallTimeline[day][1])
        + ", -"
        + str(OverallTimeline[day][2])
        + "\n"
    )
del day

fout.write("\n\nUser Timeline\n")
fout.write("Date, ")
for user in users:
    fout.write(user + ", ")
fout.write("\n")
for day in OverallTimeline:
    fout.write(str(day) + ", ")
    for user in users:
        if day in UserTimeline[user]:
            fout.write(str(UserTimeline[user][day]) + ", ")
        else:
            fout.write("0, ")
    fout.write("\n")

fout.close()
print("Done! Output file written at", "${ProjectRoot}/build/contriStat.csv")