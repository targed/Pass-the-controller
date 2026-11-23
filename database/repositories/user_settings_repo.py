from sqlalchemy import select
from sqlalchemy.orm import Session
from database.models.user import User
from database.models.user_settings import UserSettings
from pathlib import Path
from sqlalchemy import ForeignKey





class UserSettingsRepo:

    # Create -----------------------------------------------------------------------
    def create_userSettings(self, session: Session) -> UserSettings:
        query = select(User)
        local_user = session.scalar(query)

        newUserSettings = UserSettings(user_id = local_user.user_id, display_name = "Guest", notification_prefs = 1, audio_volume = 50) # Default values

        session.add(newUserSettings)
        return newUserSettings
     

    # Read -----------------------------------------------------------------------

    def get_local_settingsID(self, session: Session) -> UserSettings | None:
        query = select(UserSettings)
        local_settings = session.scalar(query)

        return local_settings.user_id
    

    def get_local_settingsDisplayName(self, session: Session) -> UserSettings | None:
        query = select(UserSettings)
        local_settings = session.scalar(query)

        return local_settings.display_name
    

    def get_local_profilePicture(self, session: Session) -> UserSettings | None:
        query = select(UserSettings)
        local_settings = session.scalar(query)
        
        return local_settings.profile_picture
    
    def get_local_notificationPrefs(self, session: Session) -> UserSettings | None:
        query = select(UserSettings)
        local_settings = session.scalar(query)

        return local_settings.notification_prefs
    
    def get_local_audioVolume(self, session: Session) -> UserSettings | None:
        query = select(UserSettings)
        local_settings = session.scalar(query)

        return local_settings.audio_volume
    

    # Update -----------------------------------------------------------------------

    def update_local_settingsID(self, session: Session, newUserID: str) -> UserSettings | None:
        query = select(UserSettings)
        local_settings = session.scalars(query).first()
        
        if local_settings is None:
            return
        

        local_settings.user_id = newUserID
        session.commit()

        return local_settings
    

    def update_local_displayName(self, session: Session, newDisplayName: str) -> UserSettings | None:
        query = select(UserSettings)
        local_settings = session.scalars(query).first()
        
        if local_settings is None:
            return
        

        local_settings.display_name = newDisplayName
        session.commit()

        return local_settings
    


    def update_local_notificationPrefs(self, session: Session, newPref: int) -> UserSettings | None:
        query = select(UserSettings)
        local_settings = session.scalars(query).first()
        
        if local_settings is None:
            return
        

        local_settings.notification_prefs = newPref
        session.commit()

        return local_settings
    

    def update_local_audioVolume(self, session: Session, newVolume: int) -> UserSettings | None:
        query = select(UserSettings)
        local_settings = session.scalars(query).first()
        
        if local_settings is None:
            return
        

        local_settings.audio_volume = newVolume
        session.commit()

        return local_settings


    # Delete -----------------------------------------------------------------------

    def delete_local_profilePicture(self, session: Session) -> bool:
        query = select(UserSettings)
        local_settings = session.scalars(query).first()
        
        if local_settings is None:
            return False
        

        local_settings.profile_picture = None
        session.commit()

        return True

        

