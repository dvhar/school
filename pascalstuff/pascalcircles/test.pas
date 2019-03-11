program test;

uses
 Sysutils, StrUtils;

Type
  circle = Object
  num_checks : integer;
  num_arrows : integer;
  arrows : array of integer;
end;

var
  circles: array of circle;  { array of circle objects }
begin
  writeln('|':10, 'Max in Circ |':15, 'Avg in Circ |':15,'Total Checks |':15,'Total Circs |':15);
  writeln('---------+--------------+--------------+--------------+--------------+');
  writeln(1:6,'   |', 54:8,' |':7, 54:8,' |':7, 334:8,' |':7, 5:8,' |':7);
  writeln('running');
end.
