var fs = require("fs");
var concat = require("concatenate-files");

var data = JSON.parse(fs.readFileSync("compile-data.json"));

concat(data.sources, data.target, { separator: "\n" }, function(err, result) {} );
