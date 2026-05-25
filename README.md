# kal - calendar program

kal is a re-imagining of the BSD calendar program, somewhat expanded in terms of capability.

## Installation

Move the resulting binary anywhere into your $PATH (e.g., /usr/local/bin or ~/bin).

## Calendar File Format (~/.kalendar)

The data file is located at ~/.kalendar and processed line-by-line. Empty lines are ignored.

1. Comment Lines: Lines beginning with a hash mark (#) are treated as comments. They are silently skipped unless verbose mode (-v) is enabled.

2. Event Lines: Entries must consist of a date or conditional designator, followed immediately by a literal tab character (\\t), followed by the event payload. Trailing whitespace on the designator side is automatically sanitized. Lines missing a tab delimiter are rejected.

3. Continuation Lines: Lines beginning with a literal tab character (\\t) inherit the designator condition parsed from the most recent valid event line.

## Sample File Configuration

This is a sample file.  Each of the day/date formats ignore case and recognize most spellings, delimiters, and abbreviations, as described in Designator Syntax Reference, below.

- *\there is a line that prints every day
- \there is a continuation line that also prints every day
- may *\there is a line that prints every day of May (case is ignored)
- 5 *\there is a line that prints every day of May using numeric format
- 5/25\there is a line that prints every May 25th
- may 25, 2026\there is a line that only prints on May 25, 2026
- mon\there is a line that only prints on Mondays
- weekday\there is a line that only prints on Monday through Friday
- weekend\there is a line that only prints on the weekend
- 4th mon\there is a line that only prints on the fourth Monday of the month
- new moon\there is a line that only prints on the new moon
- full moon\there is a line that only prints on the full moon

## Designator Syntax Reference

Textual matching across all designators is completely case-insensitive:

| Designator Syntax	|Type|	Match Condition / Examples |
|---|---|---|
|*|	Global Wildcard	|Matches every single day of the year.|
|MMM *|	Textual Month Wildcard	|Matches every day of that month. Supports 3-letter (jan), 4-letter (sept), and full names (september). |

|MMM DD	|Exact Text Date	|Matches a specific day annually (e.g., may 25, Jan 01).|
|M DD / M/DD / M-DD	|Exact Numeric Date	|Matches a specific day annually using numbers (e.g., 5 25, 5/25, 5-25).|
|MMM DD YYYY / MMM DD, YYYY	|Year-Bound Absolute Date	|Strictly matches only on that specific day and year (e.g., may 25 2026).|
|sun, monday, tue, ...	|Weekday Matcher	|Fires selectively on the matching day of the week (short or full names).|
|weekday / weekend	|Macro Blocks	|Matches Monday–Friday, or Saturday–Sunday respectively.|
|Xth WDAY	|Relative Ordinal Weekday	|Calculates and matches the occurrence week of a weekday (e.g., 4th mon, 4th monday).|
|new moon / full moon	|Lunar Phase Markers	|Employs an inline Conway-influenced synodic calculation window (\\\pm1 day tolerance).|

## Usage & Core Pipelines

| Usage | Output | Invocation |
|---|---|---|
| Basic Execution| Display matching entries scheduled for today| kal |
| Chronological Sorting | Because kal prints events in stream-order as they appear in the file, place your 24-hour time values at the beginning of your event descriptions and pipe standard output straight into sort | kal \| sort |
| Tag & Priority Filtering | Extract specific priority targets or context tags downstream using grep | kal \| sort \| grep "PRIORITY_A" (OR) grep -i "todo" |

## Verbose Diagnostics
To verify your file parsed properly and see comment streams,use kal -v

## Diagnostics & Exit Status

- Exits 0 on successful execution.
- Exits 1 if the HOME environment variable is missing or if the ~/.kalendar file cannot be opened.

## Author
William Orian Wear
