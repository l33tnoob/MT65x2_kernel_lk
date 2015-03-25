This is a daemon process for power-off alarm.

since we need to decide the next step is power-off alarm or normal boot

after kernel boot on by RTC. We need to set a system property here for that decision.

it is a execute once process and it will stop very soon after the seting done.

