# base-boilerplate

The boilerplate to create projects using Chromium's base library.

## Usage

Bootstrap:

```bash
./script/bootstrap.js
```

Change args of configuration:

```bash
./script/args.js
```

Build:

```bash
./script/build.js
```

Build a specified target:

```bash
./script/build.js base_unittests
```

Build a specified target under a configuration:

```bash
./script/build.js out/Default base_unittests
```
