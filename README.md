# base-boilerplate

The boilerplate to create projects using Chromium's `base` and `net` libraries.

## Usage

Bootstrap:

```bash
./scripts/bootstrap.js
```

Change args of configuration:

```bash
./scripts/args.js
```

Build:

```bash
./scripts/build.js
```

Build a specified target:

```bash
./scripts/build.js base_unittests
```

Build a specified target under a configuration:

```bash
./scripts/build.js out/Default base_unittests
```
