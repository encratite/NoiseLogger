set client_min_messages to warning;

drop table if exists volume_log cascade;

create table volume_log
(
	time timestamp,
	volume integer not null,
	primary key (time)
);

drop table if exists address_log;

create table address_log
(
	address text,
	last_log_time timestamp not null,
	primary key (address)
);

create or replace function drop_functions() returns void as $$
declare
	function_record record;
	drop_command text;
begin
	for function_record in
		select * from pg_proc inner join pg_namespace ns on (pg_proc.pronamespace = ns.oid) where ns.nspname = 'public' order by proname
	loop
		drop_command := 'drop function ' || function_record.nspname || '.' || function_record.proname || '(' || oidvectortypes(function_record.proargtypes) || ');';
		--raise warning '%', drop_command;
		execute drop_command;
	end loop;
end $$ language 'plpgsql';

select drop_functions();

create function insert_volume(volume_time timestamp, volume integer) returns void as $$
begin
	begin
		insert into volume_log
			(time, volume)
		values
			(insert_volume.volume_time, insert_volume.volume);
	exception when unique_violation then
	end;
end $$ language 'plpgsql';