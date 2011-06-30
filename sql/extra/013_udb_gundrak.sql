-- 7363 should be kept as disabled for the moment
delete from achievement_criteria_requirement where criteria_id IN (7319, 7583) and type = 11;

insert into achievement_criteria_requirement values
(7319, 18, 0, 0),
(7583, 18, 0, 0),
(7363, 18, 0, 0);

insert ignore into spell_script_target values
(54878, 1, 29573),
(55127, 1, 29713);