# Dataloss prevention


## Checksum choice:

Siphash.

Alternate was blake3 but it seemed not worth the trouble.

## Overall design:

For every file we extract a list of checksums.

These are the blocks in that file, and the blocks are determined by the blockizing function which is determined by the language.

We will also extract checksums for every line if we expect many edits and want a higher level of detail.

Now, clearly, if the checksums match it is highly likely that the block contents are the same but by the pigeonhole principle it cannot be certain.

We are only using a 64-bit hash.

However, if we have a hash of the lines, we can be as precise as we want and as certain as we want.

Note that a file is now just a list of checksums.

We treat the checksum as a reliable index into the blocks.

## Basic operations:

Block hash operation.

Type: Span -> u64.

This is siphash.

An index is simply a column vector of a table indexed by block index or by hash.

What we already have is a map from blocks to bytes, and to bytes of an original file as well.

So we index the blocks in a file to a location in bytes in memory.
The differences between this byte and the file offset gives us the bytes on disk.

## UX

The definition of data loss that is relevant is a user-centric one.
Data is lost if the user cannot find it, which means the key should be data visibility and searchability.

Versions of a function or block are simply block revs with a short edit distance from it.
In other words a "version" of something might be an independent reinvention, a copy of a common third source, etc.

The basic operations I want are:

- Show me where I am working (this means, which blocks are hot).
- Show me what this block has recently looked like and what has changed.
- Possibly, show me all the blocks that have differed from this earlier version that worked.

By the way, when we do a (B)uild and the build command succeeds, it succeeds on a set of blocks.
The checksum of these is as good as a git hash.

This implies that we can do git bisect and so on, but on the finer level of revs not commits.

## Implementation of the basic operations

Show me where I am working:

We have a list of the revisions already (e.g. the revs have timestamps as the filenames).

From this we can get a list of block checksums in each rev.

A good way to understand embeddings is to understand a useful cryptographic hash function as the perfect anti-embedding.
It's as bad as an embedding can be, because to the extent that it succeeds as a cryptographic hash, it perfectly fails to convey any useful information about the input, except for identity, which it turns out is useful all by itself.

In any case, we can get the blocks that changed in the files themselves but we don't have similarity data yet.

The best way to say which blocks have recently changed is to print the tops of the blocks with the newest checksums.

We can have a map from checksums to revs.
Every rev which contains a block having that checksum can get added to this index.

Show me what this block has recently looked like and what has changed:

These features have to wait until we have a similarity measure on block contents.
Otherwise we would have to do heuristics on nearby blocks and I'm not interested in writing that code only to throw it away later.

However, as a stopgap to give us both this and any other diff/patch features we might want, we can have a simple operation that can be used as a building block:

diff(span, span)

But there is another, better stopgap that better illustrates the point of our 64b embedding or hash.

We can take the count of line hashes that match and get a direct similarity measure on blocks.
This is good enough to give some rudimentary undo features, etc.

Another point that may be easier to implement immediately is a per-file undo since revs map to files anyway.

A final point is that when we are saving a rev coming from an "e" or "r/R" we already know what block it was replacing, and it's always (for now, since we don't have v support) just one block.
