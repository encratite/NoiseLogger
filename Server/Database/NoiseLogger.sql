drop table if exists volume cascade;

create table volume
(
	time timestamp,
	mean smallint
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

create function insert_mean(mean_time timestamp, mean integer) returns void as $$
declare
	smallint_maximum integer := 32767;
	compressed_mean smallint;
begin
	if mean > smallint_maximum then
		compressed_mean := smallint_maximum;
	else
		compressed_mean := mean::smallint;
	end if;
	begin
		insert into volume
			(time, mean)
		values
			(insert_mean.mean_time, compressed_mean);
	exception when unique_violation then
	end;
end $$ language 'plpgsql';