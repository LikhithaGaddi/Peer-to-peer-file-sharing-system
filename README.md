# Peer-to-peer-file-sharing-system

g++ tracker.cpp -o tracker -pthread -lssl -lcrypto
./tracker

g++ peer.cpp -o peer -pthread
./peer 127.0.0.1 6000 tracker_info.txt

###### Commands​:
- Tracker​:
  - Run Tracker: ./tracker​ tracker_info.txt ​tracker_no tracker_info.txt - Contains ip,port details of all the trackers
  - Close Tracker: quit
- Client​:
  - Run Client: ​./client​ <IP>:<PORT> tracker_info.txttracker_info.txt - Contains ip, port details of all the trackers
  - Create User Account:​ create_user​ <user_id> <passwd>
  - Login: ​login ​<user_id> <passwd>
  - Create Group:​ create_group​ <group_id>
  - Join Group:​ join_group​ <group_id>
  - Leave Group:​ leave_group​ <group_id>
  - List pending join: ​requests list_requests ​<group_id>
  - Accept Group Joining Request: ​accept_request​ <group_id> <user_id>
  - List All Group In Network:​ list_groups
  - List All sharable Files In Group:​ list_files​ <group_id>
  - Upload File: ​upload_file​ <file_path> <group_id​>
  - Download File:​ download_file​ <group_id> <file_name> <destination_path>
  - Logout:​ logoutn.Show_downloads: ​show_downloads
  - Stop sharing: ​stop_share ​<group_id> <file_name>
