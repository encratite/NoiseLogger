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

create function insert_volume(volume_time timestamp, volume integer, address text) returns void as $$
declare
	utc_timestamp timestamp;
	rows_affected integer;
begin
	select (now() at time zone 'utc') into utc_timestamp;
	begin
		insert into volume_log
			(time, volume)
		values
			(insert_volume.volume_time, insert_volume.volume);
	exception when unique_violation then
	end;
	update address_log
		set last_log_time = utc_timestamp
		where address_log.address = insert_volume.address;
	get diagnostics rows_affected = row_count;
	if rows_affected = 0 then
		insert into address_log
			(address, last_log_time)
		values
			(insert_volume.address, utc_timestamp);
	end if;
end $$ language 'plpgsql';