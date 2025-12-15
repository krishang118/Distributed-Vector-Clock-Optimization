#!/usr/bin/env python3

import re
from datetime import datetime
def parse_log_line(line):
    pattern = r'Process(\d+) (executes internal event|sends message|receives) ([em]\d+(?:\s+(?:to|from) process\d+)?) at ([\d:.]+), vc: \[([\d\s]+)\]'
    match = re.match(pattern, line)
    if not match:
        return None
    process = int(match.group(1))
    action = match.group(2)
    event_info = match.group(3)
    timestamp = match.group(4)
    vc_str = match.group(5)
    vc = [int(x) for x in vc_str.split()]
    return {
        'process': process,
        'action': action, 'event_info': event_info,
        'timestamp': timestamp, 'vc': vc,
        'line': line.strip()}
def compare_vc(vc1, vc2):
    less_equal = all(vc1[i] <= vc2[i] for i in range(len(vc1)))
    greater_equal = all(vc1[i] >= vc2[i] for i in range(len(vc1)))
    if less_equal and not all(vc1[i] == vc2[i] for i in range(len(vc1))):
        return 'less'  # vc1 < vc2
    elif greater_equal and not all(vc1[i] == vc2[i] for i in range(len(vc1))):
        return 'greater'  # vc1 > vc2
    elif not less_equal and not greater_equal:
        return 'concurrent'  # vc1 || vc2
    else:
        return 'equal'
def verify_consistency(log_file):
    print(f"\nVerifying consistency for {log_file}")
    print("\n")
    events = []
    with open(log_file, 'r') as f:
        for line in f:
            event = parse_log_line(line)
            if event:
                events.append(event)
    print(f"Total events parsed: {len(events)}")
    send_events = {}
    receive_events = {}
    for event in events:
        if 'sends message' in event['action']:
            match = re.search(r'm(\d+)', event['event_info'])
            if match:
                msg_id = (event['process'], int(match.group(1)))
                send_events[msg_id] = event
        elif 'receives' in event['action']:
            match = re.search(r'm(\d+)', event['event_info'])
            sender_match = re.search(r'from process(\d+)', event['event_info'])
            if match and sender_match:
                sender = int(sender_match.group(1))
                msg_id = (sender, int(match.group(1)))
                receive_events[msg_id] = event
    print(f"Send events: {len(send_events)}")
    print(f"Receive events: {len(receive_events)}")
    print("\nVerifying Property 1: Send -> Receive (x -> y => vh < vk)")
    print("\n")
    violations = 0
    verified = 0
    for msg_id, send_event in send_events.items():
        if msg_id in receive_events:
            recv_event = receive_events[msg_id]
            comparison = compare_vc(send_event['vc'], recv_event['vc'])
            if comparison == 'less':
                verified += 1
                if verified <= 5: 
                    print(f"Message {msg_id}: Send VC {send_event['vc']} < Receive VC {recv_event['vc']}")
            else:
                violations += 1
                print(f"VIOLATION: Message {msg_id}: Send VC {send_event['vc']} NOT < Receive VC {recv_event['vc']}")
    if verified > 5:
        print(f"... and {verified - 5} more verified pairs")
    print(f"\nProperty 1 Results: {verified} verified, {violations} violations")
    print("\nVerifying Property 2: Concurrent Events (x || y <=> vh || vk)")
    print("\n")
    concurrent_count = 0
    checked = 0
    for i in range(min(len(events), 100)):
        for j in range(i+1, min(len(events), 100)):
            e1 = events[i]
            e2 = events[j]
            if e1['process'] == e2['process']:
                continue
            checked += 1
            comparison = compare_vc(e1['vc'], e2['vc'])
            if comparison == 'concurrent':
                concurrent_count += 1
                if concurrent_count <= 5: 
                    print(f"Concurrent: P{e1['process']}-{e1['event_info'][:10]} {e1['vc']} || P{e2['process']}-{e2['event_info'][:10]} {e2['vc']}")
    if concurrent_count > 5:
        print(f"... and {concurrent_count - 5} more concurrent pairs found")
    print(f"\nProperty 2 Results: Found {concurrent_count} concurrent event pairs out of {checked} checked")
    print("\nVerifying Monotonicity: Within each process, VCs increase")
    print("\n")
    process_events = {}
    for event in events:
        p = event['process']
        if p not in process_events:
            process_events[p] = []
        process_events[p].append(event)
    monotone_violations = 0
    for process, p_events in process_events.items():
        for i in range(len(p_events) - 1):
            vc1 = p_events[i]['vc']
            vc2 = p_events[i+1]['vc']
            if vc1[process-1] >= vc2[process-1]:
                monotone_violations += 1
                print(f"VIOLATION in Process{process}: VC at event {i} is {vc1}, next is {vc2}")
    if monotone_violations == 0:
        print("All processes maintain monotonically increasing vector clocks.")
    print(f"\nMonotonicity Results: {monotone_violations} violations")
    print("\n")
    print("Verification Completed.")
    print("\n")
if __name__ == "__main__":
    import sys
    verify_consistency("vector_clock_log.txt")
    print("\n\n")
    verify_consistency("sk_vector_clock_log.txt")
