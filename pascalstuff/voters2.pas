
program votes;

uses
 Sysutils, StrUtils, Crt;

const
  G = 'G';
  P = 'P';
  O = 'O';
var
  {voters: array[0..24] of string = (G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,P,P,P,P,P,P,P,P,P,P);}
  voters: array[0..6,0..6] of string = ((O,O,O,O,O,O,O),(O,G,G,G,G,G,O),(O,G,G,G,G,G,O),(O,G,G,G,G,G,O),(O,P,P,P,P,P,O),(O,P,P,P,P,P,O),(O,O,O,O,O,O,O));
  greenwins: array[0..5] of integer = (0,0,0,0,0,0);
  NumberOfSchemesGenerated: integer = 1000000;


procedure run();
begin
  writeln('running');
end;





begin
  Randomize;
  run();
end.
