exported([test:bar/3, test:foo/4]).

bar(X, Y, Z)    :- X=1, Y=2, Z=3.
foo(X, Y, Z, W) :- X=1, Y=2, Z=3, W=4.

