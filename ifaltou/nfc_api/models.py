from django.db import models

# Create your models here.
class NFCReading(models.Model):
    tag_id = models.CharField(max_length=255, unique=True)
    timestamp = models.DateTimeField(auto_now_add=True)

    def __str__(self):
        return f'Tag ID: {self.tag_id} - Timestamp: {self.timestamp}'
    
    
class Meta:
    app_label = 'nfc_api'