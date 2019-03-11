
program dice;

uses
 Sysutils, StrUtils;

var
  input: integer;
  i: integer;
  die: integer;
  sum: integer = 0;
  avg: real;
  frames: array of string;
begin
  Randomize;
  writeln('How many dice rolls?');
  readln(input);

  if (input > 1000) then begin
    writeln('too many');
    exit;
  end;

  if (input < 1) then begin
    writeln('no roles');
    exit;
  end;

  for i := 0 to input do begin
    die := random(6)+1;
    sum := sum + die;
  end;
  avg := sum / input;
  writeln('average: ',avg:0:2);
  {writeln('average: ',FormatFloat('0.00',avg));}
end.

