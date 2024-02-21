from rest_framework import serializers
from .models import NFCReading

class NFCSerializer(serializers.ModelSerializer):
    class Meta:
        model = NFCReading
        fields = '__all__'