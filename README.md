# Intro to emscripten (Porting C code to the Web) Step 1

Web Assembly can be a great way to port or use C / C++ code to the Web.  This repository is a linear set of commits that function as a step-by-step tutorial through how to take C code and run it in a browser.  To navigate to the different steps, view or checkout the git tags wich will all be prefixed with `step_##` where `##` is the order in which this tutorial is meant to be read.

## The code we are porting

`size.c`

```c
#include <math.h>
#inlcude <emscripten.h>

EMSCRIPTEN_KEEPALIVE
double size(double x, double y) {
  return sqrt(x * x + y * y);
}
```

We have a function that calculates the length of an 2-d vector. Notice the `EMSCRIPTEN_KEEPALIVE` macro.  This is what tells the compiler to make sure this is exported in wasm and therefore callable from JavaScript.  The other way we could do this would be passing an argument to the compiler `-s "EXPORTED_FUNCTIONS=['_size']" but this is less intuitive from the code and gets verbose when we start exporting more functions.

***NOTE*** This function is a horrible idea in production.  The overhead for loading a Web Assembly module most likely outweighs the extra efficienty you might get when make the computation.  The above function can be replaced by `Math.sqrt(Math.pow(x, 2) + Math.pow(y, 2))` which most likely calls back to C brwoser functions anyway.  *This is for learning purposes only*.  These ideas should be applied to more complex libraries and apps taht need heavy or customized computation..

## How to compile

```bash
$ emcc -s "ENVIRONMENT=web" size.c -o size.js
```
* `-s "ENVIRONMENT=web"` - by default emscripten outputs a javascript file that will work with NodeJS and the Web.  This tells emscripten we only want this ti work in a browser environment.  This helps reduce code size.
* `size.c` - Our source code
* `-o size.js` - Tells the compiler to output JavaScript 'glue' code as well as a WASM file

## How to call C code from Javascript

Now we have a `.wasm` file as well as an emscripten runtime (the JavaScript "glue").  Now we need to actualy execure the WASM file in the browser and modify a webpage.  

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