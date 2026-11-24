from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.user import User
from database.models.known_peer import KnownPeer
from sqlalchemy import ForeignKey
from datetime import datetime, timezone

# ===========================================================================
# FOR NETWORKING:
# The known peer table needs to have a user's public key, which is only accessible through networking logic. You must obtain a user's publc key and pass it into this function in order to use it.
# ===========================================================================

class KnownPeerRepo:

    # Create -----------------------------------------------------------------------
    def create_knownPeer(self, session: Session, peerPublicKey: bytes) -> KnownPeer:
        query = select(User)
        local_user = session.scalar(query)

        if local_user is None:
            raise ValueError("ERROR: create_knownPeer() FAILED, a user must exist first")

        current_time = datetime.now(timezone.utc).isoformat()

        newPeer = KnownPeer(owner_user_id = local_user.user_id, peer_public_key = peerPublicKey, first_seen = current_time, last_seen = current_time)

        session.add(newPeer)
        return newPeer

    

    # Read -----------------------------------------------------------------------

    def get_peer_publicKey(self, session: Session) -> bytes | None:
        query = select(KnownPeer.peer_public_key)
        return session.scalar(query)
    

    def get_peer_firstSeen(self, session: Session, peerPublicKey: bytes) -> str | None:
        query = select(KnownPeer.first_seen).where(KnownPeer.peer_public_key == peerPublicKey)
        peer_firstSeen = session.scalar(query)
        return peer_firstSeen
    
    def get_peer_lastSeen(self, session: Session, peerPublicKey: bytes) -> str | None:
        query = select(KnownPeer.last_seen).where(KnownPeer.peer_public_key == peerPublicKey)
        peer_lastSeen = session.scalar(query)
        return peer_lastSeen

    

    # Update -----------------------------------------------------------------------

    def update_peer_lastSeen(self, session: Session, peerPublicKey: bytes) -> str | None:
        query = select(KnownPeer).where(peerPublicKey == KnownPeer.peer_public_key)
        peer_lastSeen = session.scalar(query)

        if peer_lastSeen is None:
            return
        
        peer_lastSeen.last_seen = datetime.now(timezone.utc).isoformat()
        session.commit()

        return peer_lastSeen.last_seen


    # Delete -----------------------------------------------------------------------


    def delete_knownPeer(self, session, peerPublicKeyToRemove: bytes) -> bool:
        query = select(KnownPeer).where(peerPublicKeyToRemove == KnownPeer.peer_public_key)
        pkey = session.scalar(query)

        if pkey is None:
            return False
        
        session.delete(pkey)
        session.commit()
       
        return True






