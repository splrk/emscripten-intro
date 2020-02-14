# Intro to emscripten (Porting C code to the Web) Step 2

Web Assembly can be a great way to port or use C / C++ code to the Web.  This repository is a linear set of commits that function as a step-by-step tutorial through how to take C code and run it in a browser.  To navigate to the different steps, view or checkout the git tags wich will all be prefixed with `step_##` where `##` is the order in which this tutorial is meant to be read.

## The code we are porting

We are going to work on porting a library to help up analyze files for the EBU R 128 loudness standard.  This is a standard scale for measuring how loud audio content sounds to human ears. Instead of just measuring the peak decibal at a given point, EBU R 128 measures the density of the sound.  If your interested you can read more here https://www.iconnectivity.com/blog/2017/6/10/ebu-r128-the-important-audio-breakthrough-youve-never-heard-of

This step focuses on compiling a C library and using it from JavaScript.  The big idea we need to understand here is pointers.  In this step we are going to exute a airly simple function: `ebur128_get_version`.

### Adding an external library

This project used git subtrees to inlcude the `https://github.com/jiixyj/libebur128` library.  If you take a look at the docs you'll see that compiling this code requires CMake and GNU Make.  These programs are an environment configuration tool and a build automation tool respectively.  You will need to [Install CMake](https://cmake.org/install/) and [Install GNU Make](https://www.gnu.org/software/make/).

Emscripten provides two tools to ensure that `configure`, `cmake` and `make` work: `emconfigure` and `emmake`.  To get started `cmake` needs to be invoked on the libebur128 library and then `make` needs to be invokes to create a binary that emscripten can work with.  Here are the commands that you will need to run:

```bash
$ mkdir libebur128/build
$ cd libebur128/build
$ emconfigure cmake ..
$ emmake make
```

The final command will output a binary file `libebur/build/libebur128.a`.  This can now be passed into `emcc` to give us our web Assembly and JavaScript files.

### Compiling

```bash
$ emcc -s "ENVIRONMENT=web" -s "EXPORTED_FUNCTIONS=['_ebur128_get_version']" libebur128/build/libebur128.a -o public/ebur128.js
```

* `-s "ENVIRONMENT=web"` - by default emscripten outputs a javascript file that will work with NodeJS and the Web.  This tells emscripten we only want this ti work in a browser environment.  This helps reduce code size.
* `-s "EXPORTED_FUNCTIONS=['_ebur128_get_version','_malloc']"` - since we are not providing our own code with `EMSCRIPTEN_KEEPALIVE` annotations, we have to tell emscripten whitch functions to keep from the command line interface.  I'll explain why we need to export `_malloc` later.
* `libebur128/build/libebur128.a` - The LLVM bit code to use.
* `-o public/ebur128.js` - Tells the compiler to output JavaScript 'glue' code as well as a WASM file

For convenience and reference there are two npm build scripts to compile code. The first is `npm run build:dev` which includes the `-g` flag for debug symbols.  The other is `npm run build` which builds with optimizations enabled. `emconfigure cmake` is not captured in the npm scripts so that will need to be run manually.

## How to call C code from Javascript

Now we have a `.wasm` file as well as an emscripten runtime (the JavaScript "glue").  Now we need to actualy execute the WASM file in the browser and display the results in the DOM.

```html
...
      <script type="text/javascript">
        let x = 3;
        let y = 4;
        document.getElementById('x').innerText = x.toString();
        document.getElementById('y').innerText = y.toString();

        var Module = {
          onRuntimeInitialized: () => {
            let size = Module._size(x, y);
            document.getElementById('len').innerText = size;
          }
        }
      </script>
      <script src="./size.js"></script>
...
```

First we need to declare an `onRuntimeInitialized` function.  Emscripten's javascript glue code downloads and compiles our WASM module.  When everything is ready it calls the `onRuntimeInitialized` if it exists.  When the C runtime is initialized, we can then call functions exported from our module. Just take note of the `_` in front of the name of the function.  All C functions javascript will be modifed like this.

## Running the example

This repository uses [Browser Sync](https://www.browsersync.io/) to serve the wasm file temporarily.  Even though the project is front-end only code, The WASM file still needs to be fetched via `XMLHttpRequests` to load the binary file into an `ArrayBuffer`.  Since `fetch` and `xhr` cannot request local files fo security reasons, Our binary file needs to be served up.  Browser sync is an easy wayto do this, plus it builds in auto-refreshing.  Anytime you compile the module or change the html `browser-sync` will referest your browser ans show you the latest version of your app. Here is the script to get going,but you can also just run `npm start` from this repository.

```javascript
const browserSync = require('browser-sync');

const bs = browserSync.create();

bs.watch('public/**/*.html', bs.reload);
bs.watch('public/**/*.js', bs.reload);
bs.watch('public/**/*.wasm', bs.reload);

bs.init({
  server: './public',
});
```

# More

To keep learning more about emscripten, check out more tags in this repo.