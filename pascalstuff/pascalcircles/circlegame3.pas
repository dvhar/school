{*
  David Hardy   2/27/19  "cs4500: Introduction to the Software Profession"

  Program explanation:
    This program loads 3 files specified by the user with circles that point to each
    other and then randomly traverses them, marking each step with a 'check' until all 
    have been marked. It prints the results as a table, then saves the table to a text
    file and exits.

  Input and output files:
    -3 input text files prompted from the user
    -HW3hardyOutfile.txt:  Output file with game results.

  If the input file describes a graph that is not strongly connected, or is otherwise 
    unusable, it will prompt the user for another one.

  Data structures:
    -circle: object used to represent each circle. Contains the number of arrows, list of 
     arrows, and number of checks recieved.
    -circles: array of circle objects. The name of each circle is represented by its index 
     in the array rather than as a member of the object. Index 0 is unused.
    -results: object used to return the results of a single game. Contains the total number 
     of checks in the game and the maximum number of checks in one circle.
    -visited: array of booleans used by the connectedness verifyier to keep track of which 
     circles are reachable from any given circle.
    -table_data: array of strings containing stats to put into the table
*}

program circlegame;

uses
 Sysutils, StrUtils;

{ circle type with:
    -number of checks aquired during the game.
    -number of arrows pointing to other circles. 
    -list of indexes pointing to other circles }
Type
  circle = Object
  num_checks : integer;
  num_arrows : integer;
  arrows : array of integer;
end;

{ results type with:
    -number of total checks aquired during the game.
    -maximum number of checks in one circle }
Type
  results = Object
  total_checks : longint;
  max_checks : longint;
end;


{ global variables }
var
  outfile_name: string = 'HW3hardyOutfile.txt';  { output file name }
  get_infile_name: string;  { output file name gotten from user }
  outfile: TextFile;     { output file }
  num_circles: integer = 0;   { total number of circles in one game }
  total_circles: integer = 0;   { total number of circles overall }
  total_checks: integer = 0;   { total number of checks overall }
  total_arrows: integer = 0;   { total number of arrows overall }
  num_arrows: integer = 0;   { total number of arrows in one game }
  circles: array of circle;  { array of circle objects }
  visited: array of boolean; { used to verify connectedness }
  table_data: array[0..20] of string; { result data to print to the table }


{ display error message when given its corresponding number }
procedure errorMessage(problem: integer);
var
  err: string;
begin
  { determine which error message to use }
  case (problem) of
    1 : err := 'Input file is not formatted correctly.';
    2 : err := 'Number of arrows in file does not match number specified on second line.';
    3 : err := 'Too many arrows.';
    4 : err := 'Input file does not exist.';
    5 : err := 'Arrow points to non-existant circle.';
    6 : err := 'Graph is not strongly connected.';
  end;
  { display the error message }
  writeln(err);
end;

{ recursive procedure finds all circles reachable from this circle.
  global vars: visited, circles}
procedure verifyCircle(i: integer);
var
  j: integer;
begin
  { mark current circle as reachable }
  visited[i] := true;
  { mark circles pointed to by arrows as reachable }
  for j := 0 to circles[i].num_arrows-1 do begin
    if not visited[circles[i].arrows[j]] then begin
      verifyCircle(circles[i].arrows[j]);
    end
  end
end;

{ verify graph is strongly connected.
  global vars: visited, num_circles }
function verifyGraph(): boolean;
var
  i: integer;
  j: integer;
begin

  verifyGraph := true;
  { loop through each circles and make sure it can reach all the others }
  setlength(visited, num_circles+1);
  for i := 1 to num_circles do begin

    { initialize visted array to all false }
    for j := 1 to num_circles do begin
      visited[j] := false;
    end;
    { call recursive verifier function on each circle }
    verifyCircle(i);
    { see if any circles could not be reached }
    for j := 1 to num_circles do begin
      if visited[j] = false then begin
        errorMessage(6);
        verifyGraph := false;
      end
    end;
  end;
end;

{ update 'circles' array with new circles and arrows when given a line from the input file.
  global vars: circles, num_circles }
procedure getArrow(line: string);
var
  arrow: integer;   { destination circle of arrow }
  i: integer;  { the arrow's origin circle's index }
begin
  { load and validate input }
  if not TryStrToInt( ExtractWord(1,line,[' ']), i ) or
  not TryStrToInt( ExtractWord(2,line,[' ']), arrow ) then
    errorMessage(1);
  if arrow > num_circles then
    errorMessage(5);

  { add arrow to existing circle }
  if (length(circles) > i) and (circles[i].num_arrows > 0) then begin
    inc(circles[i].num_arrows);
    setlength(circles[i].arrows, circles[i].num_arrows);
    circles[i].arrows[circles[i].num_arrows-1] := arrow;
    end

  { load new circle into array. Resize it if necessary }
  else begin
    if i >= length(circles) then
      setlength(circles, i+1);
    setlength(circles, num_circles);
    circles[i].num_arrows := 1;
    setlength(circles[i].arrows,1);
    circles[i].arrows[0] := arrow;
  end;
end;

{ load data from file into the appropriate variables and the circles array.
  return value: boolean confirming file has loaded correctly.
  global vars:  infile_name, num_circles, num_arrows }
function loadFile(infile_name: string): boolean;
var
  i: integer = 0; { line counter }
  infile: TextFile; { input file }
  line: string; { stores one line at a time from the file }
  label end_load; { used to skip the parts that require that the file exists if it does not }
begin
  loadFile := true;
  { make sure input file exists and open it }
  if not FileExists(infile_name) then begin
      errorMessage(4);
      loadFile := false;
      goto end_load;
  end;
  AssignFile(infile, infile_name);
  try
    reset(infile);

  { get number of circles from first line }
  readln(infile, line);
  if not TryStrToInt(line, num_circles) then begin
      errorMessage(1);
      loadFile := false;
  end;
  setlength(circles, num_circles+1);

  { get number of arrows from second line }
  readln(infile, line);
  if not TryStrToInt(line, num_arrows) then begin
      errorMessage(1);
      loadFile := false;
  end;

  { loop through the rest of input file and load cicles into array }
  while not eof(infile) do
  begin
    readln(infile, line);
    getArrow(line);
    inc(i);
  end;
  CloseFile(infile);
  except
    on E: EInOutError do begin
      writeln('Bad input: ',E.ClassName, '/', E.Message);
      loadFile := false;
      end
  end;

  { make sure the number of arrows is within specifications }
  if (loadFile) and (i <> (num_arrows)) then begin
      errorMessage(2);
      loadFile := false;
  end;
  if i > (num_circles * 20) then begin
      errorMessage(3);
      loadFile := false;
  end;
  end_load:

end;

{ play the game and return object with results.
  return value: result object with the total checks and max checks in a circle.
  global vars: circles, num_arrows, num_circles }
function game(): results;
var
  num_marked: integer = 0;  { keep track of how many circles have been visited }
  marker: integer;  { array index for the current circle }
  res: results; { return object with game results }
begin
  { initialize vars and start game at circle 1 }
  res.total_checks := 0;
  res.max_checks := 0;
  marker := 1;

  while num_marked < num_circles do begin
    {writeln('marker at ',marker);}
    { see if marker is at a new circle }
    if circles[marker].num_checks = 0 then
      inc(num_marked);
    { add check to circle and check for new max checks per circle }
    inc(circles[marker].num_checks);
    if circles[marker].num_checks > res.max_checks then
      res.max_checks := circles[marker].num_checks;
    inc(res.total_checks);
    { return from function if there are too many checks }
    if res.total_checks > 999999 then begin
      writeln('too many checks');
      Break;
    end;
    { randomly pick an arrow to the next circle }
    marker := circles[marker].arrows[random(circles[marker].num_arrows)];
  end;
  game := res;

end;

{ play 3 games and calculate some statistics. print and save the results
  global vars: circles, num_circles, num_arrows }
procedure playGames();
var
  i : integer = 0;
  games_played : integer = 0;
  max_checks_per_game : integer = 0;
  avg_checks_per_game : real = 0.0;
  max_single_checks: integer = 0;
  res: results; { results returned from each game }
  label get_input; { used to ask for input again if file or graph is bad }
begin
  Randomize;
  while games_played < 3 do begin
  
    { prompt for input file and verify. Repeat if not good. }
    get_input:
    setlength(circles, 0);
    writeln('Enter an input file name:');
    readln(get_infile_name);
    if not loadFile(get_infile_name) or not verifyGraph() then
      goto get_input;

    { run the game and save the results to the table_data array }
    res:= game();
    table_data[5*games_played] := IntToStr(num_circles);
    table_data[5*games_played+1] := IntToStr(num_arrows);
    table_data[5*games_played+2] := IntToStr(res.total_checks);
    table_data[5*games_played+3] := FloatToStrF(res.total_checks/num_circles, ffFixed,4, 2);
    table_data[5*games_played+4] := IntToStr(res.max_checks);

    { record some stats for the last row of the table }
    total_checks := total_checks + res.total_checks;
    total_circles := total_circles + num_circles;
    total_arrows := total_arrows + num_arrows;
    avg_checks_per_game := avg_checks_per_game + res.total_checks;

    { check for new maximum in a circle }
    if res.max_checks > max_single_checks then
      max_single_checks := res.max_checks;

    { check for new maximum in a circle }
    if res.total_checks > max_checks_per_game then
      max_checks_per_game := res.total_checks;

    { clear all checks from circles }
    for i := 1 to num_circles do begin
      circles[i].num_checks := 0;
    end;
    inc(games_played);
  end;

  { add stats for 'total' row to table_data array after 3 games are done }
  table_data[15] := IntToStr(total_circles);
  table_data[16] := IntToStr(total_arrows);
  table_data[17] := IntToStr(total_checks);
  table_data[18] := FloatToStrF(1.0*total_checks/total_circles, ffFixed,4, 2);
  table_data[19] := IntToStr(max_single_checks);

  { print results to screen, starting with table header }
  writeln('         +--------------+--------------+--------------+--------------+--------------+');
  writeln('|':10, 'Total Circles |':15, 'Total Arrows |':15, 'Total Checks |':15, 'Avg in Circ |':15, 'Max in Circ |':15);
  writeln('---------+--------------+--------------+--------------+--------------+--------------+');
  { print the rest of the rows }
  for i := 0 to 3 do begin
    case i of 
      3: write('| Total':5);
      else write('|',i+1:6);
    end;
    writeln('  |', table_data[i*5]:8,' |':7, table_data[i*5+1]:10,' |':5, table_data[i*5+2]:8,' |':7,table_data[i*5+3]:8,' |':7, table_data[i*5+4]:8,' |':7);
    writeln('---------+--------------+--------------+--------------+--------------+--------------+');
  end;

  { write results to file. Repeat code because it's faster than figuring out string formatting }
  AssignFile(outfile, outfile_name);
  rewrite(outfile);
  { write table header }
  writeln(outfile,'         +--------------+--------------+--------------+--------------+--------------+');
  writeln(outfile,'|':10, 'Total Circles |':15, 'Total Arrows |':15, 'Total Checks |':15, 'Avg in Circ |':15, 'Max in Circ |':15);
  writeln(outfile,'---------+--------------+--------------+--------------+--------------+--------------+');
  { write the rest of the  rows }
  for i := 0 to 3 do begin
    case i of 
      3: write(outfile,'| Total':5);
      else write(outfile,'|',i+1:6);
    end;
    writeln(outfile,'  |', table_data[i*5]:8,' |':7, table_data[i*5+1]:10,' |':5, table_data[i*5+2]:8,' |':7,table_data[i*5+3]:8,' |':7, table_data[i*5+4]:8,' |':7);
    writeln(outfile,'---------+--------------+--------------+--------------+--------------+--------------+');
  end;
  CloseFile(outfile);
  writeln('Saved results to ',outfile_name);

end;

{ run the program and wait to close so user can read output }
begin
  playGames();
  readln();
end.
