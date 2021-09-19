# ddthin
help migrating thin volumes and their snapshots

**experimental software - not tested for production use**
*no warranty (see license) - if you delete your disk it is your problem*

## Backstory
`ddthin` is a tool I developed when trying to migrate a set of thin-LVs with their snapshots from one thin-pool to the other. At the time I was unable to find astandard solution for moving a thin-LV with snapshots to another thinpool without copying metadata volumes which also would have required me to move the entire thin-pool.

The problem is maintaining the space-saving COW features of a thin snapshot. `ddthin` is simply a tool that reads from one file/blockdevices and writes the bytes blockwise to another, but before a block is written `ddthin` checks if the block is already present on the destination. This way *sparseness* is maintained as well as shared blocks between LVs.

If someone knows a better way to migrate thinLV between thinpools with standard tools, please let me know.

## Strategy for migrating a thin-LV and its snapshot
- create target thin-pool
- create empty LV the same size of originLV
- `ddthin /dev/oldvg/volume1 /dev/newvg/volume1`
- create snapshot of newvg/volume1
- `ddthin /dev/oldvg/volume1_snap /dev/newvg/volume1_snap`

Creating the empty LV first will initialize it with zeroes and `ddthin`'ing will maintain sparseness by not copying zero-blocks.
When migrating the thin snapshot it is first necessary to create a snapshot inside the target-VG so that all shared blocks are first marked as such. Using `ddthin` then on the existing new snapshot will ensure only the different blocks are written in the thinpool and this way the advantage of having shared blocks is ensured.

## Bugs and Issues
This software is highly experimental, so there are bound to be some bugs in various edge-cases. `ddthin` tries to keep data intact, even if source and target have different sizes. If blocksizes differ then the last block might be written incompletely. If the source is bigger than the destination `ddthin` will only copy up to the last fillable block in the destination.

## TODO
- Enable support for stdin/stdout.
