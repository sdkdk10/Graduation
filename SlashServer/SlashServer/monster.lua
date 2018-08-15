myid  =999999;

function set_myid(x)
 myid =x; 
end

function LoadMonsterData()
my_x = math.random(0,500)
my_z = math.random(0,500)
my_hp = 60;
my_dmg = 10;
return my_x, my_z, my_hp, my_dmg
end