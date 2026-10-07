# The container classes and the parsed INI file

Step 1 of `CONTAINERS_HANDOFF.md`: the class family under the INI reader, mapped from the disassembly and the RTTI export
(`tools/rtti.tsv`). **Names are the original's** (RTTI type descriptors), except member and method names, which are ours.
The game's string class is called `SqzString` in RTTI; our reconstruction calls it `String` (`GameString.h`).

## Summary
- Plain C++ templates with **ordinary single and multiple inheritance**. No virtual inheritance, no vbptr, no vbtables.
  MSVC's layout here is the same as GCC's (primary base at 0, second base right after it).
- The "dictionary" is a **sorted array with binary search**, not a tree or a hash table (`Set<T,ID>`).
- Keys compare with the case-insensitive `String` compare (`StringLess` @4386f0, used as `operator<`).
- Destructors are **not virtual** in any of these classes, so the vtables hold only the methods listed below.
- The last template argument (`ID`, a small number) is a type tag for the save/load system (`BufferObject`). It does not
  change any code; every `ID` gets its own copy of the same functions (no identical-code folding in the original link),
  which is why the binary has runs of same-sized functions.
- The INI path uses: `Array<String>`, `PrimitiveArray<String,3>`, `Set<String,23/50/51>`,
  `Table<String,String,PrimitiveArray<String,3>,23>` + `BufferObjectTable<...>`, `Table<String,IniFileSection*,...,50>`,
  `Table<String,IniFileCache*,...,51>`, `Array<IniFileSection*>`, `Array<IniFileCache*>`, and (`GetIntList`) `PrimitiveArray<int,1>`.
- The game calls `dynamic_cast` (`__RTDynamicCast`, 33 call sites), which reads the RTTI locator at `vtable[-1]`. So any
  container object that original code can see must carry the **original vtable address**, not one made by our compiler.

## Inheritance
```
                 Array<T>                               BufferObject  (abstract: 4 pure virtuals, vtable 0x49d52c)
   vptr, count, capacity, data   (0x10)                 vptr          (0x4)
                    |                                       |
                    +-------------------+-------------------+
                                        |
                          PrimitiveArray<T, ID>          (0x14)  Array at +0, BufferObject at +0x10
                                        |
                                  Set<T, ID>             (0x14)  sorted, unique; adds 3 virtuals

  Table<K, V, VArray, ID>        (vptr + Set<K,ID> keys + VArray values; no base class)
            |
  BufferObjectTable<K, V, VArray, ID> : Table, BufferObject   (BufferObject right after the Table)

  IniFileSection   (no vtable)   { BufferObjectTable<String,String,PrimitiveArray<String,3>,23> values; PrimitiveArray<String,3> order; }
  IniFileCache     (no vtable)   { Table<String,IniFileSection*,Array<IniFileSection*>,50> sections; }   one parsed file
  global @4c7738                 Table<String,IniFileCache*,Array<IniFileCache*>,51>                      file name -> parsed file
```
`IniFileSection` and `IniFileCache` have RTTI type descriptors (they appear as `IniFileSection*` in the table names) but no
vtable of their own, and their constructors only construct the members, so they are plain classes that **contain** the
containers (if they derived from them MSVC would have emitted their own vtables). Sure, from the layouts and the inlined
construction.

## Array&lt;T&gt;  (abstract in spirit; never instantiated alone)
Size 0x10. `+0 vptr`, `+4 count`, `+8 capacity`, `+0xc data` (heap array of `capacity` elements).

| slot | method (our name) | String copy | int copy | what it does |
|---|---|---|---|---|
| 0 | `T Get(int i)` | 0x40ec10 | 0x40ec80 | returns `data[i]` by value |
| 1 | `int Insert(const T &v, int at)` | 0x45e280 | 0x45d920 | `Grow(1)`; `at == -1` appends, else shifts up and stores at `at`; returns the index |
| 2 | `void Remove(int i)` | 0x45e300 | 0x45d990 | shifts the tail down, `count--` (the old last slot keeps its value) |
| 3 | `void Set(int i, const T &v)` | 0x45e340 | 0x45d9d0 | `data[i] = v` |
| 4 | `T *Data()` | 0x45e360 | 0x45d9f0 | |
| 5 | `int Grow(int extra)` | 0x45e370 | 0x45da00 | if `count + extra >= capacity`: capacity *= 1.5 (`cap + cap>>1`, once), copy, free old; returns 1 if it grew |

Constructor (count 0, capacity 2): String 0x45e1b0, int 0x45d8d0, pointer types inlined. Copy constructor (String) 0x45e0e0.
Destructor (non-virtual): String 0x45e250, int 0x45d900, `IniFileSection*` 0x46f350, `IniFileCache*` 0x46f3d0.
Storage: for `String` the original uses `new String[n]` / `delete[]` (game `operator new` 0x48ea00 with a count word in
front, the CRT's vector constructor/destructor iterators 0x48eb80 / 0x48ea50); for `int` and pointers a plain
`operator new(n * 4)` / `operator delete`. The String copies of slots 0-5 are shared by every `PrimitiveArray<String,N>` and
`Set<String,N>` (they are inherited, not per ID). Each pointer element type has its own copy (e.g. `Array<IniFileSection*>`
0x46d400 0x46dc60 0x46dcd0 0x46dd10 0x46dd30 0x46dd40; `Array<IniFileCache*>` 0x46e090 0x46e900 0x46e970 0x46e9b0 0x46e9d0 0x46e9e0).
Vtables: `Array<String>` 0x4a000c, `Array<int>` 0x49ffcc, `Array<IniFileSection*>` 0x4a0d6c, `Array<IniFileCache*>` 0x4a0e1c.

## BufferObject  (vtable 0x49d52c, all four slots the pure-virtual stub 0x48e4d0)
The save/load interface. Slots, from the `PrimitiveArray<String,3>` copies: 0 type name (0x45e490, builds "Array" plus the
ID with `sprintf("%2ux")` into a too-small buffer, an original bug), 1 create a new empty object of the same class (0x45e520),
2 write to a stream (0x45e680), 3 read from a stream (0x45e620). **Not used anywhere on the INI path**; not reconstructed.

## PrimitiveArray&lt;T, ID&gt; : Array&lt;T&gt;, BufferObject
Size 0x14, `BufferObject` at +0x10. No members of its own and no methods of its own beyond the four BufferObject slots.
Constructor always inlined: `Array()` then `[+0x10] = 0x49d52c`, `[+0] = PrimitiveArray vtable`, `[+0x10] = its BufferObject
vtable`. Destructor: `~Array` (e.g. 0x40be40 for ID 3, 0x46f4e0 for 23, 0x46f760 for 50, 0x46f7b0 for 51).

| class | vtable (+0) | vtable (+0x10) |
|---|---|---|
| `PrimitiveArray<String,3>` | 0x49d50c | 0x49d4f4 |
| `PrimitiveArray<String,23>` | 0x4a0334 | 0x4a031c |
| `PrimitiveArray<String,50>` | 0x4a0cf4 | 0x4a0cdc |
| `PrimitiveArray<String,51>` | 0x4a0da4 | 0x4a0d8c |
| `PrimitiveArray<int,1>` | 0x49d5a4 | 0x49d58c |

## Set&lt;T, ID&gt; : PrimitiveArray&lt;T, ID&gt;  (sorted, unique keys)
Size 0x14. Overrides two slots and adds three:

| slot | method | ID 23 | ID 50 | ID 51 | what it does |
|---|---|---|---|---|---|
| 1 | `int Insert(const T &v, int hint)` | 0x462380 | 0x46d640 | 0x46e2d0 | keeps the array sorted: uses `hint` when it is a valid position, else a binary search; an equal key is overwritten in place; returns the index |
| 3 | `void Set(int i, const T &v)` | 0x4627e0 | 0x46daa0 | 0x46e740 | `data[i] = v` (first compares the old value with `v` and throws the result away, a disabled check) |
| 6 | `int Contains(const T &v)` | 0x4626f0 | 0x46d9b0 | 0x46e650 | `IndexOf(v) != -1` |
| 7 | `int IndexOf(const T &v)` | 0x462710 | 0x46d9d0 | 0x46e670 | binary search, -1 when missing |
| 8 | `void RemoveKey(const T &v)` | 0x4627c0 | 0x46da80 | 0x46e720 | `Remove(IndexOf(v))` when present |

Vtables (+0 / +0x10): ID 23 0x4a036c / 0x4a0354, ID 50 0x4a0d2c / 0x4a0d14, ID 51 0x4a0ddc / 0x4a0dc4. Constructor inlined
(inside the `Table` constructors). Destructor: ID 23 0x46eeb0, 50 0x46f2f0, 51 0x46f370.
The same 831-byte `Insert` exists for every ID (0x45f760 ID 11, 0x460930 21, 0x461710 22, 0x462380 23, 0x46d640 50; 0x46e2d0 51
differs only in register choice), plus `Set<int,ID>` and `Set<float,25>` copies.

## Table&lt;K, V, VArray, ID&gt;  (key -> value map: two parallel arrays)
`+0 vptr`, `+4 Set<K,ID> keys`, `+0x18 VArray values` (index i of `values` belongs to key i of `keys`).
Size 0x2c with `PrimitiveArray` values, 0x28 with `Array<T*>` values.

| slot | method | String->String (23) | ->IniFileSection* (50) | ->IniFileCache* (51) | what it does |
|---|---|---|---|---|---|
| 0 | `int Insert(const K &k, const V &v, int hint)` | 0x462bc0 | 0x46de30 | 0x46ead0 | `had = keys.Contains(k)`, `i = keys.Insert(k, hint)`, then `values.Set(i, v)` if it had the key else `values.Insert(v, i)`; returns i |
| 1 | `int Contains(const K &k)` | 0x462c20 | 0x46de90 | 0x46eb30 | |
| 2 | `int IndexOf(const K &k)` | 0x462c40 | 0x46deb0 | 0x46eb50 | |
| 3 | `void RemoveAt(int i)` | 0x462c60 | 0x46ded0 | 0x46eb70 | removes from both arrays |
| 4 | `void Remove(const K &k)` | 0x462c90 | 0x46df00 | 0x46eba0 | `RemoveAt(IndexOf(k))` when present |
| - | `V Get(const K &k)` (not virtual) | 0x462cc0 | 0x46df30 | 0x46ebd0 | `values[keys.IndexOf(k)]`, **no check for -1** (callers test `Contains` first) |

Vtables: 0x4a0394, 0x4a0d54, 0x4a0e04. Constructors: 0x4629a0, 0x46d2c0, 0x46df50. Destructors: 0x462b50, 0x46ddc0, 0x46ea60.
`Get` for the String table goes through a helper 0x40f2a0 -> 0x40f310 (`values[i]` by value).

`BufferObjectTable<String,String,PrimitiveArray<String,3>,23>`: `Table` plus `BufferObject` at +0x2c, size 0x30. Vtables
0x49d574 (+0, same five slots as the Table) and 0x49d55c (+0x2c, serialisation). Constructor inlined (calls the Table
constructor 0x4629a0), destructor 0x40c3d0.

## The INI classes
| | what | size | address |
|---|---|---|---|
| `IniFileSection` | one `[section]`: `values` (key -> value, sorted) at 0, `order` (keys in file order) at 0x30 | 0x44 | constructor and destructor inlined |
| `IniFileSection::Add(char *line, const char *section)` | splits `key = value`, trims blanks, warns on a duplicate key ("PARSING ERROR! duplicate entry", the new value wins), stores it, appends the key to `order` | | 0x442730 |
| `IniFileSection::Find(const char *key)` | value text or NULL (the text stays owned by the table) | | 0x4428a0 |
| `IniFileSection::Keys()` | `&order` | | 0x442980 |
| `IniFileCache` | one parsed file: `sections` (section name -> `IniFileSection*`) | 0x28 | constructor inlined (`Table` ctor 0x46d2c0) |
| `IniFileCache::~IniFileCache()` | deletes every section, removing it from the table, last first | | 0x4429e0 |
| `IniFileCache::Load(const char *path)` | the parser; logs "PARSING INI %s..." | | 0x442aa0 |
| `IniFileCache::Keys(const char *section)` | the section's `order` list or NULL | | 0x442e20 |
| `IniFileCache::Find(const char *section, const char *key)` | value text or NULL (`IniFind` in `Original.h`) | | 0x442ee0 |
| `IniOpen(const char *path)` | the cached file, parsed on first use and kept forever (also when the file is missing: an empty file object is cached) | | 0x442fa0 |
| `IniCloseAll()` | deletes every cached file and empties the cache | | 0x4430e0 (via 0x443260) |
| `g_iniFiles` | the cache: `Table<String,IniFileCache*,Array<IniFileCache*>,51>` | 0x28 | 0x4c7738, static ctor 0x4429a0 -> 0x46df50, atexit dtor 0x4429c0 -> 0x46ea60 |
| `IniReader::GetKeys()` | new `PrimitiveArray<String,3>` with the current section's keys in file order | | 0x443470 (27 callers) |
| `IniReader::GetIntList(key)` | `ParseIntList(Lookup(key))`; crashes when the key is missing (no NULL check) | | 0x443350 |
| `IniReader::ParseIntList(text)` | new `PrimitiveArray<int,1>` of the space separated ints (atoi) | | 0x4435f0 |

Parser details (0x442aa0): `fgets` 255 bytes; strips one trailing `\n`; skips blank lines; a `;` cuts the line, and a line
whose first non-blank character is `;` is skipped; `[name]` (blanks allowed inside the brackets, the name ends at the first
blank or `]`) starts a section: a new `IniFileSection` is created, an existing section of the same name is **deleted and
replaced** (its keys are lost); lines with `=` go to the current section (ignored before the first section header). The
helpers 0x4426d0 (skip blanks), 0x4426f0 (back up over blanks), 0x442710 (find the next blank) treat space and tab as blanks.

## Where these are called from
- `IniOpen`: the two `IniReader` constructors (ours). `IniFind`: `IniReader::Lookup` (ours). `Keys`: `IniReader::GetKeys`.
- `IniCloseAll` (0x443260 -> 0x4430e0): called from 0x4130b0, a `LocationSimulation` vtable slot (leaving a location), so it
  **does** run in play, not only at shutdown.
- `Array<String>::Insert` 0x45e280 has 22 direct callers all over the game; the other container code is reached through
  vtables, which `CALL_MAP.md` does not count.

## How sure
Sure (read from the code): every layout, offset, size, slot meaning and address above; the inheritance (RTTI base lists agree
with the constructor sequences); sorted array and case-insensitive keys; the INI class layouts.
Guessed: method names (ours); that `ID` is a save/load type tag (it appears only in the type-name slot); the exact
`BufferObject` slot meanings (only partly read, not needed now).

## Plan for the source (step 2)
- One template each (`Array`, `PrimitiveArray`, `Set`, `Table`, `BufferObjectTable`) in `Container.h`, with real `virtual`
  methods declared in the original slot order (GCC then uses the same slot numbers), and `IniFile.h/.cpp` for the INI classes.
- Original code uses these objects and runs `dynamic_cast`, so every constructor ends by pointing the object at the
  **original** vtable(s) (a table of addresses per instantiation in `Original.h`). Virtual calls from our code then go through
  the original vtable to the original functions, which the hooks redirect to ours.
- Two GCC/MSVC differences to respect: GCC passes the hidden result pointer of a by-value class return (`String Get(int)`)
  in `ecx` and `this` on the stack, MSVC the other way round, so our code never calls `Get` virtually on a String array
  (it calls `Array<T>::Get` directly; no class overrides it). And a `bool` return sets only `al`, so methods the original
  code tests with `test eax,eax` return `int`.
- `BufferObject` stays a bare vtable pointer (`void **vtbl`) until save/load is reconstructed, so the serialisation slots
  are not needed now.
