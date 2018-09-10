myid  =999999;

NPC_SPIDER = 0;
NPC_NAGA_GUARD = 1;
NPC_ROCK_WARRIOR = 2;
NPC_TREE_GUARD = 3;
NPC_TURTLE = 4;
NPC_DRAGON = 5;

function set_myid(x)
 myid =x; 
end

function LoadMonsterData(npcType)
if(npcType == NPC_SPIDER) then
my_x = math.random(30,105)
my_z = math.random(-7,8)
my_hp = 5;
my_dmg = 1;
elseif(npcType == NPC_NAGA_GUARD) then
my_x = math.random(79,95)
my_z = math.random(10,48)
my_hp = 5;
my_dmg = 1;
elseif(npcType == NPC_ROCK_WARRIOR) then
my_x = math.random(96,147)
my_z = math.random(58,83)
my_hp = 5;
my_dmg = 1;
elseif(npcType == NPC_TREE_GUARD) then
my_x = math.random(96,147)
my_z = math.random(58,83)
my_hp = 5;
my_dmg = 1;
elseif(npcType == NPC_TURTLE) then
my_x = math.random(96,147)
my_z = math.random(58,83)
my_hp = 5;
my_dmg = 1;
end
return my_x, my_z, my_hp, my_dmg
end