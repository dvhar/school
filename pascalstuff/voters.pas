
program voters;

uses
 Sysutils, StrUtils, Crt;

const
  G = 'G';
  P = 'P';
var
  votes: array[0..24] of string = (G,G,G,G,G,G,G,G,G,G,G,G,G,G,G,P,P,P,P,P,P,P,P,P,P);
  greenwins: array[0..5] of integer = (0,0,0,0,0,0);
  NumberOfSchemesGenerated: integer = 1000000;

procedure randarray();
var i: integer;
var rand: integer;
var temp: string;
begin
  for i := 0 to 24 do begin
    rand := random(25);
    temp := votes[i];
    votes[i] := votes[rand];
    votes[rand] := temp;
  end
end;


procedure contigious();
var i: integer = 0;
var rand: integer;
var rand: integer;
var temp: string;
var avail: array of integer;
begin
  rand := random(25);
  if (rand % 5 == 0)
    avail
end;


procedure run();
var 
  i,j,k: integer;
  GreenMajorityDistricts: integer;
  GreenBlocks: integer;
  voterIndex: integer;
begin
  for i := 0 to NumberOfSchemesGenerated do begin
    randarray();
    GreenMajorityDistricts := 0;
    for j := 0 to 4 do begin
      GreenBlocks := 0;
      for k := 0 to 4 do begin
        voterIndex := (5*j) + k;
        if votes[voterIndex] = G then
          inc(GreenBlocks);
      end;
      if GreenBlocks >= 3 then
        inc(GreenMajorityDistricts);
    end;
    inc(GreenWins[GreenMajorityDistricts]);
  end;
  writeln('Results from simulations, showing how many schemes result in Green wins:');
  for i := 0 to 5 do begin
    writeln(i, '  wins for Green,  ', 5-i, '  wins for Purple: ', GreenWins[i]);
  end
end;





begin
  Randomize;
  run();
end.
