from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.user import User
from database.models.known_peer import KnownPeer
from database.models.host_ban import HostBan
from sqlalchemy import ForeignKey
from datetime import datetime, timezone

# ===========================================================================
# FOR NETWORKING:
# The host ban table needs to have a user's public key, which is only accessible through networking logic. You must obtain a user's publc key and pass it into this function in order to use it.
# ===========================================================================

class HostBanRepo:

    # Create -----------------------------------------------------------------------

    def create_hostBan(self, session: Session, peerPublicKey: bytes, bannedReason: str) -> HostBan:
        query_for_id = select(User)
        local_user = session.scalar(query_for_id)

        query_for_key = select(KnownPeer).where(KnownPeer.peer_public_key == peerPublicKey, KnownPeer.owner_user_id == local_user.user_id)
        knownPeerKey = session.scalar(query_for_key)

        if knownPeerKey is None:
            raise ValueError("Cannot ban a peer that is not in KnownPeer")




        current_time = datetime.now(timezone.utc).isoformat()

        newHostBan = HostBan(owner_user_id = local_user.user_id, peer_public_key = knownPeerKey.peer_public_key, banned_at = current_time, ban_reason = bannedReason)

        session.add(newHostBan)
        return newHostBan

    # Read -----------------------------------------------------------------------
    def get_bannedAt(self, session: Session, peerPublicKey: bytes) -> HostBan:
        query = select(HostBan).where(peerPublicKey == HostBan.peer_public_key)
        banned_time = session.scalar(query)
        return banned_time.banned_at
    

    def get_bannedReason(self, session: Session, peerPublicKey: bytes) -> HostBan:
        query = select(HostBan).where(peerPublicKey == HostBan.peer_public_key)
        why_banned = session.scalar(query)
        return why_banned.ban_reason


    # Update -----------------------------------------------------------------------

    def update_bannedReason(self, session: Session, peerPublicKey: bytes, newReason: str) -> HostBan | None:
        query = select(HostBan).where(peerPublicKey == HostBan.peer_public_key)
        why_banned = session.scalar(query)


        if why_banned is None:
            return False

        
        why_banned.ban_reason = newReason
        session.commit()

        return why_banned


    # Delete -----------------------------------------------------------------------

    def delete_hostBan(self, session: Session, peerPublicKeyToRemove: bytes) -> bool:
        query = select(HostBan).where(peerPublicKeyToRemove == HostBan.peer_public_key)
        pkey = session.scalar(query)

        if pkey is None:
            return False
        
        session.delete(pkey)
        session.commit()
       
        return True
