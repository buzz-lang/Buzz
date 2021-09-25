# REST-Like API
The puprose of this code is to provide a way of mimicking a REST API.
Messages/queries are received from neighbors on selected topics, and handlers are associated to those topics.
In this case, the topics and handlers are organised in a way that emulates a REST API.

First, we define a Query class:

```ruby
Query = {

    #
    # Creates a new Query.
    # PARAM source: The Query source (a robot's ID).
    # PARAM data: The Query data.
    # RETURN: A new Query.
    #
    .new = function(source, data) {
        return { 
            .source = source,
            .data = data
        }
    }
}
```

Then, we can define the query handling logic:

```ruby
include "query.bzz"


# Store topic-handler associations in a table.
# The keys should correspond to the topic names.
var query_types = { 
    .get = query_get,
    .put = query_put,
    .post = query_post,
    .delete = query_delete
}

#
# Binds topics with their handlers
#
function init_handlers() {
    foreach(query_types, function(query_type, handler) {
        neighbors.listen(query_type, function(vid, query, neighbor_id) {
            handler(query)
        })
    })
}

#
# Handles GET queries
# PARAM query: The Query to be handled.
#
function query_get(query) {
    log("GET from ", query.source, ": ", query.data)
}

#
# Handles PUT queries
# PARAM query: The Query to be handled.
#
function query_put(query) {
    log("PUT from ", query.source, ": ", query.data)
}

#
# Handles POST queries
# PARAM query: The Query to be handled.
#
function query_post(query) {
    log("POST from ", query.source, ": ", query.data)
}

#
# Handles DELETE queries
# PARAM query: The Query to be handled.
#
function query_delete(query) {
    log("DELETE from ", query.source, ": ", query.data)
}

```

Lastly, to test our system, we include it in the standard Buzz execution loop. Here, robot 0 will broadcast a PUT query to all its neighbors at every time step.

```ruby
include "query.bzz"
include "api.bzz"


#
# Called upon initialization
#
function init() {
    init_handlers()
}


#
# Called at every time step
#
function step() {
    if (id == 0) {
        neighbors.broadcast("put", Query.new(id, "This is a message."))
    }
}


#
# Called upon end of experiment
#
function destroy() {
    log("Experiment over.")
}
```