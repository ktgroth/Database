# DataFrame + B‑Tree – A Mini C “Database” Library  

A tiny C library that demonstrates how a **dataframe** (a simple in‑memory table) and a **B‑tree** can be implemented from scratch.  
The library is intentionally small, written in pure ANSI‑C (C99), and is meant as a learning/reference project.  

> **⚠️ Status** – The implementation is *not* feature‑complete.  
>  Many functions in the source tree are still TODO.  The README explains what is present, how to build and run the demo, and how to use the API that is available.

---

## Table of contents  

- [Features](#features)  
- [Directory structure](#directory-structure)  
- [Prerequisites](#prerequisites)  
- [Build & Run](#build--run)  
- [API overview](#api-overview)  
- [Examples](#examples)  
- [Future work](#future-work)  
- [Contributing](#contributing)  
- [License](#license)  

---

## Features  

| Feature | Status |
|---------|--------|
| **Dataframe** – a simple row‑based in‑memory table | ✅ |
| **Datablock** – a generic “row” container with heterogeneous column types | ✅ |
| **B‑Tree** – generic key/value index (int/float/string) | ✅ |
| **Table abstraction** – can expose either a dataframe or a B‑tree (TODO) | ⚠️ |
| **Search & Update** – based on primary key or column lookup | ✅ (basic) |
| **Custom comparison functions** – per column type | ✅ |

The code is heavily commented, and each module contains the main API functions.  The `main.c` file shows a usage demo that builds a dataframe, performs updates & searches, then builds a B‑tree index over a dummy “id” column.

---

## Directory structure  

```
📦 dataframe
 ├─ src
 │  ├─ main.c            # demo driver
 │  ├─ table.c           # table abstraction (incomplete)
 │  ├─ dataframe.c       # dataframe implementation
 │  ├─ datablock.c       # block (row) implementation
 │  ├─ btree.c           # generic B‑tree index
 │  ├─ utils.c
 │  └─include
 │    ├─ dataframe.h
 │    ├─ datablock.h
 │    ├─ btree.h
 │    ├─ table.h
 │    ├─ utils.h
 │    └─ logger.h
 ├─ README.md            # this file
 └─ LICENSE              # (add your license file)
```

*All source files are in the `src/` directory and public headers in `include/`.*

---

## Prerequisites  

- A C compiler that supports **C99** (e.g. `gcc`, `clang`).  

No external libraries are required.

---

## Build & Run  

```bash
# Clone the repo
git clone https://github.com/ktgroth/dataframe.git
cd dataframe

# Build the demo binary
./build.sh build

# Run the demo
./build/db
```

The demo will print:

```
=== Frame Add ===
...
=== Frame Update ===
...
=== Frame Search ===
...
=== Btree Add ===
...
```

The output demonstrates that the dataframe correctly stores rows, that updates and searches work, and that the B‑tree index can be created and traversed.

---

## API Overview  

### `dataframe_t`

| Function | Purpose |
|----------|---------|
| `init_frame` | Allocate a new dataframe with column names & types |
| `frame_add` | Append a `datablock_t` (row) to the frame |
| `frame_search` | Return a new dataframe that contains only rows matching one or more column/value pairs |
| `frame_update` | Update a specific column of a row identified by a key column/value |
| `print_frame` | Pretty‑print the frame contents |

### `datablock_t`

| Function | Purpose |
|----------|---------|
| `init_block` | Create a row container for a dataframe |
| `free_block` | Free all memory associated with a row |
| `get_column` | Retrieve a pointer to a specific column value in the row |

### `btree_t`

| Function | Purpose |
|----------|---------|
| `init_btree` | Create a new generic B‑tree (configurable order) |
| `btree_add` | Insert a key/value pair into the tree |
| `btree_search` | Retrieve a value by key |
| `print_btree` | Pretty‑print tree structure (for debugging) |

### `table_t`

The table abstraction is currently incomplete (TODO).  The intention is to allow a table to expose either a dataframe or a B‑tree based index.  Future releases will add:

- `table_add`
- `table_search`
- `table_update`
- `print_table`

---

## Examples  

```c
// Create a dataframe with 4 columns
const char *colnames[] = { "fname", "lname", "age", "salary" };
const type_e   coltypes[] = { COL_STRING, COL_STRING, COL_INT, COL_FLOAT };
dataframe_t *df = init_frame(4, colnames, coltypes);

// Create a row (datablock)
void *row1[] = { "Kasen", "Groth", (int []){ 22 }, (double []){ 65000 } };
datablock_t *block1 = init_block(df, row1);

// Add to dataframe
frame_add(df, block1);

// Search for all rows where lname == "Groth"
dataframe_t *groths = frame_search(df, 1, (const char *[]){"lname"}, (const void *[]){"Groth"});
print_frame(groths);
```

> **Tip** – Always `free_block()` your rows when you are done; the demo uses `frame_add()` which retains a reference to the block.

---

## Future work  

- Finish the `table.c` abstraction (index detection, multi‑column search, etc.).  
- Add unit tests (e.g. using CUnit or a simple assert‑based framework).  
- Support variable column counts per row.  
- Implement persistence (write dataframe to disk).  
- Add more data types (`COL_DATE`, `COL_BOOL`, etc.).  
- Provide command‑line interface for simple queries.

---

Happy coding!
