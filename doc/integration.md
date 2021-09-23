# Integrating Buzz with C and C++

## Basics: The BuzzVM is Stack-Based
The Buzz VM is stack-based. This means that every operation the VM can
perform uses elements on a stack as operands. For instance, to perform
a sum, the Buzz VM takes the two top elements in the stack, sums them
together, pops them from the stack, and then pushes the result on the
stack.

For this reason, when you want to integrate Buzz with existing C or
C++ code, the interaction between the BuzzVM and external code must
occur through the stack.

## Adding New Functions

To extend Buzz with C++, we follow the example from [this controller](examples/controller) made for ARGoS.
The point here is not the intergation with ARGoS in itself, but rather simply adding some functions, or *closures* to Buzz.
Note that the code in the example snippets below is *not* complete, but the code in the referenced files is complete.

### Defining a Closure
In this example, we create a controller for a KheperaIV robot with radiation sensing capabilities.
To do so, we define the [controller](examples/controller/src/argos/buzz_controller.h)
and some [radiation sources](examples/controller/src/argos/radiation_sources.h) as well as their associated .cpp files.
This controller contains the closures which will be exported to Buzz.

The closures we want to add are all defined and implemented in the buzz_controller .h and .cpp files respectively.
For example, we take a look at the `GetRadiationIntensity` function, which allows a robot the sense the radiation intensity for its current position:

```cpp
#include "radiation_source.h"

public:
    float GetRadiationIntensity();
```

```cpp
#include <json/json.h>
#include "buzz_controller_drone_sim.h"

float CBuzzControllerDroneSim::GetRadiationIntensity() {
    // Read the JSON file containing the radiation sources
    Json::Value radiationValues;
    Json::Reader reader;
    std::ifstream radiationFile(radiation_file_name_);
    reader.parse(radiationFile, radiationValues);

    // Get the robot's position
    int x = static_cast<int>(std::rint(m_pcPos->GetReading().Position.GetX()));
    int y = static_cast<int>(std::rint(m_pcPos->GetReading().Position.GetY()));

    // Compute the total perceived radiation at current position
    float totalRadiationIntensity = 0.0;
    for (auto source : radiationValues["sources"]){
        RadiationSource radiation = RadiationSource(source["x"].asFloat(), source["y"].asFloat(), source["intensity"].asFloat());
        totalRadiationIntensity += radiation.GetPerceivedIntensity(x, y);
    }

    return totalRadiationIntensity;
}
```

Note: For this function, we used [the jsoncpp library](http://jsoncpp.sourceforge.net)
as well as the [RadiationSource](examples/controller/src/argos/RadiationSource) class.

Several other functions (some specific to ARGoS controllers) are also included in the files as additional examples.

### Exporting a Closure
To export the previously defined closure to Buzz, we refer to the [buzz_closures.cpp](examples/controller/src/argos/buzz_closures.cpp) file.
Note that in order to interact with the Buzz VM, we use helper functions/macros like `buzzvm_pushs` which are defined in
[buzz_utils.h](examples/controller/src/utils/buzz_utils.h).

The first step is to create an intermediary function, `BuzzGetRadiationIntensity`, which will call our `GetRadiationIntensity` function.
The important point is that `BuzzGetRadiationIntensity` pops the required arguments from the Buzz VM stack
(in this case, there are none, but other functions like `BuzzLogDataSize` in the same file show how to do this),
calls the desired controller function and pushes the result onto the Buzz VM stack.

```cpp
#include "buzz_controller_drone_sim.h"
#include "utils/buzz_utils.h"

static int BuzzGetRadiationIntensity(buzzvm_t vm) {
    /* Get pointer to the controller */
    buzzvm_pushs(vm, buzzvm_string_register(vm, "controller", 1));
    buzzvm_gload(vm);
    
    /* Call the function */
    float radiationIntensity = reinterpret_cast<CBuzzControllerDroneSim*>(buzzvm_stack_at(vm, 1)->u.value)->GetRadiationIntensity();
    
    /* Push the result to the VM */
    buzzvm_pushf(vm, radiationIntensity);

    return buzzvm_ret1(vm);
}
```

The second step is to register the instermediary function to the Buzz VM.
This is done in the same [buzz_closures.cpp](examples/controller/src/argos/buzz_closures.cpp) file.

First, we give the desired name to the function. This name is what will be used to call the function
from the Buzz script, and should therefore ideally follow Buzz naming conventions (snake_case).
This name needs to be registered as a string to the VM.
Second, we push the closure to the Buzz VM, thereby associating the closure with the Buzz function name.

```cpp
buzzvm_state CBuzzControllerDroneSim::RegisterFunctions() {
    CBuzzControllerKheperaIV::RegisterFunctions();

    // Give a name to the function and register the name
    buzzvm_pushs(m_tBuzzVM, buzzvm_string_register(m_tBuzzVM, "get_radiation_intensity", 1));

    // Push the closure to the VM
    buzzvm_pushcc(m_tBuzzVM, buzzvm_function_register(m_tBuzzVM, BuzzGetRadiationIntensity));
    buzzvm_gstore(m_tBuzzVM);

    return m_tBuzzVM->state;
}
```

### Compiling and Installing the Controller
Once the previous steps have been followed, we can compile the closure.
We recommend using CMake for this. The full CMakeLists.txt is provided [here](examples/controller/CMakeLists.txt).
Again, the CMakeLists.txt contains instructions relative to ARGoS, but these are not what you should worry about.
The essential in this file is to add the C++ sources and to link the required libraries.

```cmake
set(ARGOS_BUZZ_SOURCES
    utils/buzz_utils.h  
    argos/buzz_controller.h       
    argos/buzz_controller.cpp     
    argos/buzz_closures.cpp
    argos/radiation_source.h       
    argos/radiation_source.cpp)

add_library(argos3plugin_buzz_simulator_drone SHARED ${ARGOS_BUZZ_SOURCES})
target_link_libraries(argos3plugin_buzz_simulator_drone
    argos3core_simulator
    argos3plugin_simulator_genericrobot
    argos3plugin_simulator_spiri
    argos3plugin_simulator_buzz
    buzz
    jsoncpp)
```

We can then compile and install the controller.

```sh
cd examples/controller
mkdir build && cd build
cmake ../src
make
sudo make install
```

At this point, the controller and its closure(s) should be installed and ready to use within Buzz.
