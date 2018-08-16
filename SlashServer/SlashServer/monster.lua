myid  =999999;

function set_myid(x)
 myid =x; 
end

function LoadMonsterData()
my_type = math.random(0,2)
if(my_type == 0) then
my_x = math.random(30,105)
my_z = math.random(-7,8)
elseif(my_type == 1) then
my_x = math.random(79,95)
my_z = math.random(10,48)
elseif(my_type == 2) then
my_x = math.random(96,147)
my_z = math.random(58,83)
end
my_hp = 5;
my_dmg = 1;
return my_x, my_z, my_hp, my_dmg, my_type
end