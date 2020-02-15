# Intro to emscripten (Porting C code to the Web) Step 3

Web Assembly can be a great way to port or use C / C++ code to the Web.  This repository is a linear set of commits that function as a step-by-step tutorial through how to take C code and run it in a browser.  To navigate to the different steps, view or checkout the git tags wich will all be prefixed with `step_##` where `##` is the order in which this tutorial is meant to be read.

## The code we are porting

In this step we are going to explore an alternative way of calling the same function from step 2: `ebur128_get_version`.  For reference, here is the function signature.

```c
/** \brief Get library version number. Do not pass null pointers here.
 *
 *  @param major major version number of library
 *  @param minor minor version number of library
 *  @param patch patch version number of library
 */
void ebur128_get_version(int* major, int* minor, int* patch);
```

This time we are going to reserve memory and call `ebur128_get_version` from our own custom C code instead of from JavaScript.  This code is in `ebur128_wrapper.c`

```c
#include <stdio.h>
#include <emscripten.h>
#include "libebur128/ebur128/ebur128.h"

EMSCRIPTEN_KEEPALIVE
char* get_version() {
    static char version[8];
    int major;
    int minor;
    int patch;

    ebur128_get_version(&major, &minor, &patch);
    sprintf(version, "%d.%d.%d", major, minor, patch);
    
    return version;
}
```

Let's walk through the code a bit.

First the imports
1. `#inlcude <stdio.h>` - We need this for the `sprintf` function that creates a string.
2. `#inlcude <emscripten.h>` - The need this for the `EMSCRIPTEN_KEEPALIVE` macro that exports our function
3. `#inlucde "libebur128/ebur128/ebur128.h"` - This includes our ebur128 library.  Take note of the quotes instead of the lestt-than/greater-than brackets.  The quotes tell the C compiler that this is a custom include and that this header is not found in the standard libary.

Next, the function:

First we declare a string `char version[8]` since in C strings are just a string of characters.  I chose a length of 8 characters just in case we need double digits at some point. Notice the `static` keyword.  This makes sure that `version` is allocated in heap memory, therefore being preserved after the function call.  It also ensures that version is declared only once per program execution even if `get_version` is called multiple times.

For `major`, `minor`, and `patch` I declare them as integers on the stack instead of reserving space for them on the heap.  Then to pass the address of them to `ebur128_get_version` I need to use the `&` symbol.

Instead of passing numbers back to JavaScript, I'm using sprintf to do the string building for me and then pass the string back to JavaScript.

### Compiling

First make sure you generate `libebur/build/libebur128.a` by following the instructions from [Adding an external library](https://gitlab.com/rseal/emscripten-intro/-/tree/step_02#adding-an-external-library) in step 2.

Next:

```bash
$ emcc -s "ENVIRONMENT=web" \
  -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccalll'] \
  libebur128/build/libebur128.a -o public/ebur128.js
```

* `-s "ENVIRONMENT=web"` - by default emscripten outputs a javascript file that will work with NodeJS and the Web.  This tells emscripten we only want this to work in a browser environment.  This helps reduce code size.
* `-s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']` - `ccall` is an emscripten runtime function that will call an exported method and convert some C types to JavaScript equivalents.
* `libebur128/build/libebur128.a` - The LLVM bit code from the library we want to use.
* `-o public/ebur128.js` - Tells the compiler to output JavaScript 'glue' code as well as a WASM file

Since we are creating a wrapper library and using `EMSCRIPTEN_KEEPALIVE` our compiler arguments are shorter since we don't need `-s EXPORTED_FUNCTIONS`.  This is nice if we have may exported functons.

For convenience and reference there are two npm build scripts to compile code. The first is `npm run build:dev` which includes the `-g` flag for debug symbols.  The other is `npm run build` which builds with optimizations enabled. `emconfigure cmake` is not captured in the npm scripts so that will need to be run manually.

## How to read C strings from Javascript

Web Assembly only handles integers and floats, not strings.  Therefore Web Assembly is going pass back the address of the string and leave it up to JavaScript to determine how to read the memory at that address. Fortunately, emscripten provides a runtime to handle converting a chunck of Web Assembly memory to a JavaScript string.  We could use `getValue` like we did in step 2, but this time we are going to reduce our code one function call by using `ccall`.  This allows us to call an exported function and tell the emscripten runtime to convert the return value to a Javascript Type ine one step:

```JavaScript
Module.ccall(
  // Name of the exported function (without an underscore)
  'get_version',
  // The type to convert result to.  Can be number or boolean as well
  'string`,
  // The Javascript types of the function paramaters
  [],
  // The arguments to pass into our function
  []
)
```

For this use case we are only worried about the first two arguments since we do not need to pass in parameters.  For the full working example see below:

```html
...
      <script type="text/javascript">
        var Module = {
          onRuntimeInitialized: () => {
            let version = Module.ccall(
              'get_version',
              'string',
              [],
              []
            );

            document.getElementById('version').innerText = version;
          }
        }
      </script>
      <script src="./ebur128.js"></script>
...
```

The advantages we get from approaching the code this way are that first, we have less Javascript to write and can push write our pointer logic in a language that is more versatile with memory address and pointers. And secondly, we can control the exported functions from the C code instead of having to remember to specify each one in a compiler argument.

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