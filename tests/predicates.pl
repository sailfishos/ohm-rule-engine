:- module(predicates, [success/1, failure/1, exception/1, echo/2]).

rules([success/1, failure/1, exception/1, echo/2, undefined/1]).

% always succeed
success([[success, [always, succeeds]]]).

% always fail
failure(_) :- fail.

% raise an exception
exception(A) :- A is 1 / 0.

% 'echo' any given argument back
echo(A, List) :-
        writef('echo got %w', [A]),
	List = [[echoed, [value, A]]].

