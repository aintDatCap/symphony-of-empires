--
-- Unify germany "magic" event
--

function unify_germany_test()
    if math.random(0, 100) == 0 then
        return EVENT_CONDITIONS_MET
    end
	return EVENT_CONDITIONS_UNMET
end
function unify_germany_event()
	descision = Descision:new{
		ref_name = "unify_germany_descision_0",
		name = "Yes",
		descision_fn = "unify_germany_descision_0",
		effects = "Prussia acquires all provinces that are a nuclei of Germany"
	}
	unify_germany_evhdl:add_descision(descision)
	
	descision = Descision:new{
		ref_name = "unify_germany_descision_1",
		name = "No",
		descision_fn = "unify_germany_descision_1",
		effects = "Nothing happens"
	}
	unify_germany_evhdl:add_descision(descision)
	return EVENT_DO_ONE_TIME
end
function unify_germany_descision_0()
	print('Unifying germany...')
	Nation:get("germany"):set_ideology(Ideology:get("monarchy"))
	local prov = Nation:get("germany"):get_nuclei_provinces()
	for k, v in pairs(prov) do
		print(v.ref_name)
		v:give_and_relinquish_to(Nation:get("germany"))
	end
end
function unify_germany_descision_1()
	print('Germany isn\'t going to be a thing :<')
end
unify_germany_evhdl = Event:new{
	ref_name = "unify_germany", conditions_fn = "unify_germany_test", event_fn = "unify_germany_event",
	title = "Unify germany",
	text = "Make germany with magic!"
}
unify_germany_evhdl:register()
unify_germany_evhdl:add_receivers(Nation:get("prussia"))