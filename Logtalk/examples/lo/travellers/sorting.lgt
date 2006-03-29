
:- object(quick(_Order)).

	:- info([
		author is 'Paulo Moura',
		version is 1.0,
		date is 2000/4/22,
		parnames is ['Order'],
		comment is '.',
		source is 'Example adopted from the Francis G. McCabe L&O documentation.']).

	:- public(sort/2).
	:- mode(sort(+list, -list), one).

	sort([], []).
	sort([X| L], S):-
		split(L, X, L1, L2),
		sort(L1, S1),
		sort(L2, S2),
		app(S1, [X| S2], S).

	split([], _, [], []).
	split([D| L], X, [D| L1], L2):-
		parameter(1, Order),
		Order::less(D, X),
		!,
		split(L, X, L1, L2).
	split([D| L], X, L1, [D| L2]):-
		split(L, X, L1, L2).

	app([], L, L).
	app([H| T], L, [H| T2]) :-
		app(T, L, T2).

:- end_object.



:- object(descend).

	:- info([
		author is 'Paulo Moura',
		version is 1.0,
		date is 2000/4/22,
		comment is '.',
		source is 'Example adopted from the Francis G. McCabe L&O documentation.']).

	:- public(less/2).

	less(X, Y):-
		X >= Y.

:- end_object.


:- object(natural).

	:- info([
		author is 'Paulo Moura',
		version is 1.0,
		date is 2000/4/22,
		comment is '.',
		source is 'Example adopted from the Francis G. McCabe L&O documentation.']).

	:- public(less/2).

	less(X, Y):-
		X < Y.

:- end_object.


:- object(geographic(_OX, _OY)).

	:- info([
		author is 'Paulo Moura',
		version is 1.0,
		date is 2000/4/22,
		parnames is ['OX', 'OY'],
		comment is '.',
		source is 'Example adopted from the Francis G. McCabe L&O documentation.']).

	:- public(less/2).

	less(Town1, Town2):-
		angle(Town1, Angle1),
		angle(Town2, Angle2),
		Angle1 < Angle2.

    angle(Town, Angle) :-
		Town::at(X, Y),
		parameter(1, OX),
		parameter(2, OY),
    	angle(X, Y, OX, OY, Angle).

    angle(X, Y, OX, OY, Angle) :-
		X > OX,
		Y >= OY,
		Angle is atan((Y-OY)/(X-OX)).

    angle(X, Y, OX, OY, Angle) :-
		X > OX,
		Y < OY,
		pi(Pi),
		Angle is Pi + Pi - atan((OY-Y)/(X-OX)).

    angle(X, Y, OX, OY, Angle) :-
		X < OX,
		Y >= OY,
		pi(Pi),
		Angle is Pi - atan((Y-OY)/(OX-X)).

    angle(X, Y, OX, OY, Angle) :-
		X < OX,
		Y < OY,
		pi(Pi),
		Angle is Pi + atan((OY-Y)/(OX-X)).

    angle(OX, Y, OX, OY, Angle) :- 
		Y > OY,
    	pi(Pi),
    	Angle is Pi / 2.

    angle(OX, Y, OX, OY, Angle) :-
		Y =< OY,
    	pi(Pi),
    	Angle is 1.5 * Pi.

	pi(Pi) :-
		Pi is 4.0*atan(1.0).

:- end_object.


:- object(metric(_Town)).

	:- info([
		author is 'Paulo Moura',
		version is 1.0,
		date is 2000/4/22,
		comment is '.',
		parnames is ['Town'],
		source is 'Example adopted from the Francis G. McCabe L&O documentation.']).

	:- public(less/2).

	less((Town1, _), (Town2, _)):-
		parameter(1, Town),
		Town::crow_flies(Town1, Distance1),
		Town::crow_flies(Town2, Distance2),
		Distance1 < Distance2.		

:- end_object.