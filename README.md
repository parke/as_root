# as_root - an ultra-minimalist alternative to sudo

###  About `as_root`

`as_root` is an ultra-minimalist alternative to `sudo`.

`as_root`'s command line interface differs from `sudo`.  For example, to use run `shutdown -h now` via `sudo`, you would run the command:

    $  sudo  shutdown  -h  now

Whereas with `as_root`, you simply run the command:

    $  shutdown  -h  now

In other words, you don't need to type `as_root`.  This is possible because you install `as_root` as follows:

    $  musl-gcc  -static  as_root.c  -o /tmp/as_root
    $  sudo  mkdir  /usr/as_root                         #  You may use some other directory, if you prefer.
    $  sudo  cp  /tmp/as_root  /usr/as_root/as_root
    $  sudo  chown  root:$GROUP  /usr/as_root/as_root    #  Users in $GROUP will be able to run as_root.
    $  sudo  chmod  450  /usr/as_root/as_root            #  Make as_root group-executable. 
    $  sudo  chmod  u+s  /usr/as_root/as_root            #  Set the set-user-id bit.
    $  sudo  ln  -s  as_root  /usr/as_root/shutdown      #  Create symlinks pointing to as_root.

Then you would add `/usr/as_root` to your `PATH` before other directories.

Consequently, when you run `shutdown -h now`, it is `/usr/as_root/as_root` that runs.

`as_root` will check if its `argv` matches one of the patterns specified in the `ALLOW` global variable.

If `argv` matches an `ALLOW` pattern, then `as_root` will `execv()` the command with elevated privileges.

If `argv` matches none of the `ALLOW` patterns, then `as_root` will drop privileges and still `execv()` the command with normal privileges.  (This, too, is different from `sudo`.)

I wrote `as_root` because `btrfs` (and other commands) provide status information about the system.  I wanted to access this status information as a non-root user.  In other words, I typically use `as_root` to access status information, and not to make changes to the system.

Patterns are stored in the `ALLOW` string.  The `ALLOW` string is compiled into `as_root`.  There is no configuration file.

Patterns are separated by the newline character (`'\n'`).  Arguments in a patteren are separated by one or more spaces (`' '`).

There are three special arguments that may be used in a pattern:

*  `ARG` will match any one argument.
*  `PLUS` will match one or more arguments.
*  `STAR` will match zero or more arguments.

A single `PLUS` or `STAR` should be used only at the end of a pattern.  (Using a `PLUS` or `STAR` in the middle of a pattern is not supported and will result in silent false positive pattern matches.)
